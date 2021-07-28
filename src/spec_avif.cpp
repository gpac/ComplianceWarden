#include "spec_utils_derivations.h"
#include "spec.h"
#include "box_reader_impl.h"
#include "spec_avif_utils.h"
#include <algorithm> // std::find
#include <cassert>
#include <cstring> // strcmp
#include <map>

void checkEssential(Box const& root, IReport* out, uint32_t fourcc);
std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> getItemDataOffsets(Box const& root, IReport* out, uint32_t itemID);
std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);

namespace
{
bool operator == (const Symbol& lhs, const Symbol& rhs)
{
  if(lhs.numBits != rhs.numBits)
    return false;

  if(lhs.value != rhs.value)
    return false;

  if(strcmp(lhs.name, rhs.name))
    return false;

  return true;
}

bool operator == (const std::vector<Symbol>& lhs, const std::vector<Symbol>& rhs)
{
  if(lhs.size() != rhs.size())
    return false;

  for(size_t i = 0; i < lhs.size(); ++i)
    if(!(lhs[i] == rhs[i]))
      return false;

  return true;
}

std::vector<uint32_t /*itemId*/> findAv1ImageItems(Box const& root)
{
  std::vector<uint32_t> av1ImageItemIDs;

  // Find AV1 Image Items
  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iinf"))
          for(auto& iinfChild : metaChild.children)
            if(iinfChild.fourcc == FOURCC("infe"))
            {
              uint32_t itemId = 0;

              for(auto& sym : iinfChild.syms)
              {
                if(!strcmp(sym.name, "item_ID"))
                  itemId = sym.value;
                else if(!strcmp(sym.name, "item_type"))
                  if(sym.value == FOURCC("av01"))
                    av1ImageItemIDs.push_back(itemId);
              }
            }

  return av1ImageItemIDs;
}

Box const & explore(Box const& root, uint64_t targetOffset)
{
  for(auto& box : root.children)
    if(box.position + box.size > targetOffset)
      return explore(box, targetOffset);

  return root;
}

std::vector<Symbol> getAv1CSeqHdr(const Box* av1C)
{
  std::vector<Symbol> av1cSymbols;

  bool seqHdrFound = false;

  for(auto& sym : av1C->syms)
  {
    if(!strcmp(sym.name, "seqhdr"))
      seqHdrFound = true;

    if(!strcmp(sym.name, "/seqhdr"))
      seqHdrFound = false;

    if(seqHdrFound)
      av1cSymbols.push_back(sym);
  }

  assert(!seqHdrFound);

  return av1cSymbols;
}

const Box* findAv1C(Box const& root, IReport* out, uint32_t itemId)
{
  struct Entry
  {
    int found = 0;
    const Box* box = nullptr;
  };
  std::map<uint32_t /*property index*/, Entry> av1cPropertyIndex;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco"))
              for(uint32_t i = 1; i <= iprpChild.children.size(); ++i)
                if(iprpChild.children[i - 1].fourcc == FOURCC("av1C"))
                  av1cPropertyIndex.insert({ i, { 0, &iprpChild.children[i - 1] }
                                           });

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipma"))
            {
              uint32_t localItemId = 0;

              for(auto& sym : iprpChild.syms)
              {
                if(!strcmp(sym.name, "item_ID"))
                  localItemId = sym.value;
                else if(!strcmp(sym.name, "property_index"))
                  if(localItemId == itemId)
                    for(auto& a : av1cPropertyIndex)
                      if(a.first == sym.value)
                        a.second.found++;
              }
            }

  int found = 0;
  const Box* av1C = nullptr;

  for(auto& a : av1cPropertyIndex)
  {
    if(!a.second.found)
      continue;

    found += a.second.found;

    if(!av1C)
      av1C = a.second.box;

    break;
  }

  if(found == 0)
    out->error("No av1C configuration found (expected 1)");
  else if(found > 1)
    out->error("Found %d av1C (expected 1) - for conformance, only the first associated av1C will be considered", found);

  return av1C;
}

void probeAV1ImageItem(Box const& root, IReport* out, uint32_t itemId, BoxReader& br, av1State& stateUnused)
{
  auto const spans = getItemDataOffsets(root, out, itemId);

  if(spans.empty())
  {
    out->error("Not data offset found for item %u", itemId);
    return;
  }

  int size = 0;

  for(auto span : spans)
    size += span.second;

  if(size == 0)
    return;

  std::vector<uint8_t> bytes(size);
  int offset = 0;

  for(auto span : spans)
  {
    memcpy(bytes.data() + offset, root.original + span.first, span.second);
    offset += span.second;
  }

  br.br = BitReader { bytes.data(), (int)bytes.size() };

  while(!br.empty())
  {
    auto obuType = parseAv1Obus(&br, stateUnused, false);

    if(obuType == OBU_FRAME_HEADER || obuType == OBU_REDUNDANT_FRAME_HEADER || obuType == OBU_FRAME)
      /* if compliant we now have seen the sequence header (av1C) and the frame header*/
      return;
  }
}
} // namespace

std::initializer_list<RuleDesc> rulesAvifGeneral =
{
  {
    "Section 2.1 AV1 Image Item\n"
    "The AV1 Image Item shall be associated with an AV1 Item Configuration Property.",
    [] (Box const& root, IReport* out)
    {
      auto av1ImageItemIDs = findAv1ImageItems(root);

      // AV1 Image Item shall be associated with an AV1 Item Configuration Property
      std::vector<uint32_t> av1cPropertyIndex; /* 1-based */

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipco"))
                  for(uint32_t i = 1; i <= iprpChild.children.size(); ++i)
                    if(iprpChild.children[i - 1].fourcc == FOURCC("av1C"))
                      av1cPropertyIndex.push_back(i);

      std::map<uint32_t /*itemId*/, bool> av1ImageItemIDsAssociated;

      for(auto itemId : av1ImageItemIDs)
        av1ImageItemIDsAssociated.insert({ itemId, false });

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipma"))
                {
                  uint32_t itemId = 0;

                  for(auto& sym : iprpChild.syms)
                  {
                    if(!strcmp(sym.name, "item_ID"))
                      itemId = sym.value;
                    else if(!strcmp(sym.name, "property_index"))
                      if(std::find(av1cPropertyIndex.begin(), av1cPropertyIndex.end(), sym.value) != av1cPropertyIndex.end())
                        av1ImageItemIDsAssociated[itemId] = true;
                  }
                }

      for(auto& item : av1ImageItemIDsAssociated)
        if(item.second == false)
          out->error("AV1 Image Item (ID=%u) shall be associated with an AV1 Item Configuration Property", item.first);
    }
  },
  {
    "Section 2.1\n"
    "The AV1 Image Item Data shall be identical to the content of an AV1 Sample marked as sync",
    [] (Box const& root, IReport* out)
    {
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      for(auto itemId : av1ImageItemIDs)
      {
        av1State stateUnused;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, stateUnused);

        bool showFrame = false, keyFrame = false;
        assert(br.myBox.children.empty());

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "show_frame") && (!sym.numBits || sym.value))
            showFrame = true;

          if(!strcmp(sym.name, "key_frame"))
            keyFrame = true;

          if(!strcmp(sym.name, "frame_type"))
            if(sym.value == AV1_KEY_FRAME)
              keyFrame = true;
        }

        if(!(showFrame && keyFrame))
          out->error("AV1 Sample shall be marked as sync (showFrame=%d, keyFrame=%d)", (int)showFrame, (int)keyFrame);
      }
    }
  },
  {
    "Section 2.1\n"
    "The AV1 Image Item Data shall have exactly one Sequence Header OBU.",
    [] (Box const& root, IReport* out)
    {
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      for(auto itemId : av1ImageItemIDs)
      {
        av1State stateUnused;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, stateUnused);

        assert(br.myBox.children.empty());

        int seqHdrNum = 0;

        for(auto& sym : br.myBox.syms)
          if(!strcmp(sym.name, "seqhdr"))
            seqHdrNum++;

        if(seqHdrNum != 1)
          out->error("Expected 1 sequence Header OBU but found %d", seqHdrNum);
      }
    }
  },
  {
    "Section 2.1\n"
    "The AV1 Image Item Data should have its still_picture flag set to 1.",
    [] (Box const& root, IReport* out)
    {
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      for(auto itemId : av1ImageItemIDs)
      {
        av1State stateUnused;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, stateUnused);

        assert(br.myBox.children.empty());

        for(auto& sym : br.myBox.syms)
          if(!strcmp(sym.name, "still_picture"))
            if(sym.value == 0)
              out->warning("still_picture flag set to 0");
      }
    }
  },
  {
    "Section 2.1\n"
    "The AV1 Image Item Data should have its reduced_still_picture_header flag set to 1.",
    [] (Box const& root, IReport* out)
    {
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      for(auto itemId : av1ImageItemIDs)
      {
        av1State state;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, state);

        if(!state.reduced_still_picture_header)
          out->warning("reduced_still_picture_header flag set to 0");
      }
    }
  },
  {
    "Section 2.1\n"
    "Sequence Header OBUs should not be present in the AV1CodecConfigurationBox.",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
      {
        if(box.fourcc == FOURCC("meta"))
        {
          auto av1Cs = findBoxes(box, FOURCC("av1C"));

          for(auto& av1C : av1Cs)
            for(auto& sym : av1C->syms)
              if(!strcmp(sym.name, "seqhdr"))
                out->warning("Sequence Header OBUs should not be present in the AV1CodecConfigurationBox");
        }
      }
    }
  },
  {
    "Section 2.2.1\n"
    "If a Sequence Header OBU is present in the AV1CodecConfigurationBox, it shall match the\n"
    "Sequence Header OBU in the AV1 Image Item Data.",
    [] (Box const& root, IReport* out)
    {
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      for(auto itemId : av1ImageItemIDs)
      {
        auto av1C = findAv1C(root, out, itemId);

        if(!av1C)
          continue;

        auto const av1cSymbols = getAv1CSeqHdr(av1C);

        if(av1cSymbols.empty())
          return;

        av1State stateUnused;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, stateUnused);

        auto const av1ImageItemDataSeqHdr = getAv1CSeqHdr(&br.myBox);

        if(av1ImageItemDataSeqHdr.empty())
          out->error("No Sequence Header OBU present in the AV1 Image Item Data");
        else if(!(av1cSymbols == av1ImageItemDataSeqHdr))
          out->error("The Sequence Header OBU present in the AV1CodecConfigurationBox shall match the one in the AV1 Image Item Data");
      }
    }
  },
  {
    "Section 2.2.1\n"
    "The values of the fields in the AV1CodecConfigurationBox shall match those of the\n"
    "Sequence Header OBU in the AV1 Image Item Data.",
    [] (Box const& root, IReport* out)
    {
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      for(auto itemId : av1ImageItemIDs)
      {
        AV1CodecConfigurationRecord av1cRef {};

        auto av1C = findAv1C(root, out, itemId);

        if(!av1C)
          continue;

        for(auto& sym : av1C->syms)
        {
          if(!strcmp(sym.name, "seq_profile"))
            av1cRef.seq_profile = sym.value;

          if(!strcmp(sym.name, "seq_level_idx_0"))
            av1cRef.seq_level_idx_0 = sym.value;

          if(!strcmp(sym.name, "seq_tier_0"))
            av1cRef.seq_tier_0 = sym.value;

          if(!strcmp(sym.name, "high_bitdepth"))
            av1cRef.high_bitdepth = sym.value;

          if(!strcmp(sym.name, "twelve_bit"))
            av1cRef.twelve_bit = sym.value;

          if(!strcmp(sym.name, "monochrome"))
            av1cRef.mono_chrome = sym.value;

          if(!strcmp(sym.name, "chroma_subsampling_x"))
            av1cRef.chroma_subsampling_x = sym.value;

          if(!strcmp(sym.name, "chroma_subsampling_y"))
            av1cRef.chroma_subsampling_y = sym.value;

          if(!strcmp(sym.name, "chroma_sample_position"))
            av1cRef.chroma_sample_position = sym.value;
        }

        av1State state;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, state);

        if(memcmp(&state.av1c, &av1cRef, sizeof(AV1CodecConfigurationRecord)))
          out->error("The values of the AV1CodecConfigurationBox shall match\n"
                     "the Sequence Header OBU in the AV1 Image Item Data:\n"
                     "\tAV1CodecConfigurationBox:\n%s\n"
                     "\tSequence Header OBU in the AV1 Image Item Data:\n%s\n",
                     av1cRef.toString().c_str(), state.av1c.toString().c_str());
      }
    }
  },
  {
    "Section 2.2.1\n"
    "Metadata OBUs, if present, shall match the values given in other item properties, such as\n"
    "the PixelInformationProperty or ColourInformationBox.",
    [] (Box const& root, IReport* out)
    {
      (void)root;
      (void)out; // TODO
    }
  },
  {
    "Section 2.2.1\n"
    "AV1 Item Configuration Property [...] shall be marked as essential.",
    [] (Box const& root, IReport* out)
    {
      checkEssential(root, out, FOURCC("av1C"));
    }
  },
  {
    "Section 3\n"
    "The track handler for an AV1 Image Sequence shall be 'pict'.",
    [] (Box const& root, IReport* out)
    {
      // This rule doesn't make sense as HEIF defines an Image Sequence as a track with
      // handler_type 'pict'. Let's test:
      // - when 'avis' brand is present ('should' be), there is at least one track with
      // 'pict' handler;
      // - and also contains a primary item that is an image item (here checked as av01
      // although this is not clearly specified).

      bool hasAvisBrand = false, hasOneTrackPict = false, hasPrimaryItemImage = false;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("ftyp"))
          for(auto& sym : box.syms)
            if(!strcmp(sym.name, "compatible_brand"))
              if(sym.value == FOURCC("avis"))
                hasAvisBrand = true;

      uint32_t primaryItemId = -1;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("pitm"))
              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "item_ID"))
                  primaryItemId = field.value;

      auto av1ImageItemIDs = findAv1ImageItems(root);

      if(std::find(av1ImageItemIDs.begin(), av1ImageItemIDs.end(), primaryItemId) != av1ImageItemIDs.end())
        hasPrimaryItemImage = true;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("mdia"))
                  for(auto& mdiaChild : trakChild.children)
                    if(mdiaChild.fourcc == FOURCC("hdlr"))
                      for(auto& sym : mdiaChild.syms)
                        if(!strcmp(sym.name, "handler_type"))
                          if(sym.value == FOURCC("pict"))
                            hasOneTrackPict = true;

      {
        void (IReport::* report) (const char* fmt, ...);

        if(hasAvisBrand)
          report = &IReport::error;
        else
          report = &IReport::warning;

        if(hasOneTrackPict ^ hasPrimaryItemImage)
        {
          if(hasOneTrackPict)
            (out->*report)("Track with 'pict' handler found, but no primary item that is an AV1 image item found.");
          else if(hasAvisBrand)
            (out->*report)("Primary item that is an AV1 image item found, but no track with 'pict' handler found.");
        }
      }
    }
  },
  {
    "Section 4. Auxiliary Image Items\n"
    "The mono_chrome field in the Sequence Header OBU shall be set to 1.\n"
    "The color_range field in the Sequence Header OBU shall be set to 1.",
    [] (Box const& root, IReport* out)
    {
      // contains both image items and meta primary image item of the sequence
      auto const av1ImageItemIDs = findAv1ImageItems(root);

      std::vector<uint32_t> auxImages;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iref"))
            {
              uint32_t from_item_ID = 0;

              for(auto& field : metaChild.syms)
              {
                if(!strcmp(field.name, "box_type"))
                  if(field.value != FOURCC("auxl"))
                    break;

                if(!strcmp(field.name, "from_item_ID"))
                {
                  if(std::find(av1ImageItemIDs.begin(), av1ImageItemIDs.end(), field.value) == av1ImageItemIDs.end())
                    break;
                  else
                    from_item_ID = field.value;
                }

                if(!strcmp(field.name, "to_item_ID"))
                {
                  assert(from_item_ID);

                  // TODO: move this check to ISOBMFF 8.11.1.1
                  if(field.value == 0)
                    out->warning("The to_item_ID value of 0 should not be used - ignoring");
                  else
                    auxImages.push_back(from_item_ID);
                }
              }
            }

      for(auto itemId : auxImages)
      {
        av1State state;
        BoxReader br;
        probeAV1ImageItem(root, out, itemId, br, state);

        assert(br.myBox.children.empty());

        if(!state.av1c.mono_chrome)
          out->error("The mono_chrome field in the Sequence Header OBU shall be set to 1 (item_ID=%u)", itemId);

        if(!state.color_range)
          out->error("The color_range field in the Sequence Header OBU shall be set to 1 (item_ID=%u)", itemId);
      }
    }
  },
  {
    "Section 4. Alpha Image Images\n"
    "An AV1 Alpha Image Item (respectively an AV1 Alpha Image Sequence) shall be\n"
    "encoded with the same bit depth as the associated master AV1 Image Item\n"
    "(respectively AV1 Image Sequence).",
    [] (Box const& root, IReport* out)
    {
      // contains both image items and meta primary image item of the sequence
      auto const av1ImageItemIDs = findAv1ImageItems(root);
      struct AuxImageItemIds
      {
        uint32_t master, aux;
      };
      std::vector<AuxImageItemIds> auxImages;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iref"))
            {
              uint32_t masterItemId = -1;

              for(auto& field : metaChild.syms)
              {
                if(!strcmp(field.name, "box_type"))
                  if(field.value != FOURCC("auxl"))
                    break;

                if(!strcmp(field.name, "from_item_ID"))
                {
                  if(std::find(av1ImageItemIDs.begin(), av1ImageItemIDs.end(), field.value) == av1ImageItemIDs.end())
                    break;

                  masterItemId = (uint32_t)field.value;
                }

                if(!strcmp(field.name, "to_item_ID"))
                  auxImages.push_back({ masterItemId, (uint32_t)field.value });
              }
            }

      // find Alpha Image Items
      std::vector<int> alphaPropertyIndexes;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipco"))
                {
                  if(!alphaPropertyIndexes.empty())
                  {
                    out->error("Unexpected non-empty alphaPropertyIndexes. Ignoring data.");
                    continue;
                  }

                  int propertyIdx = 1;

                  for(auto& ipcoChild : iprpChild.children)
                  {
                    if(ipcoChild.fourcc == FOURCC("auxC"))
                    {
                      std::string auxType;

                      for(auto& sym : ipcoChild.syms)
                        if(!strcmp(sym.name, "aux_type"))
                        {
                          auxType.push_back((char)sym.value);

                          if(auxType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")
                            alphaPropertyIndexes.push_back(propertyIdx);
                        }
                    }

                    propertyIdx++;
                  }
                }

      // now find ItemID from alphaPropertyIndexes
      std::vector<int> alphaItemIds;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipma"))
                {
                  uint32_t itemId = 0;

                  for(auto& sym : iprpChild.syms)
                  {
                    if(!strcmp(sym.name, "item_ID"))
                      itemId = sym.value;
                    else if(!strcmp(sym.name, "property_index"))
                      if(std::find(alphaPropertyIndexes.begin(), alphaPropertyIndexes.end(), sym.value) != alphaPropertyIndexes.end())
                        alphaItemIds.push_back(itemId);
                  }
                }

      for(auto& auxImageIds : auxImages)
      {
        if(std::find(alphaItemIds.begin(), alphaItemIds.end(), auxImageIds.aux) == alphaItemIds.end())
          continue;

        auto computeBitDepth = [&] (uint32_t itemId) -> uint32_t
          {
            av1State state;
            BoxReader br;
            probeAV1ImageItem(root, out, itemId, br, state);

            int bitDepth = 8;

            if(state.av1c.seq_profile == 2 && state.av1c.high_bitdepth)
            {
              bitDepth = state.av1c.twelve_bit ? 12 : 10;
            }
            else if(state.av1c.seq_profile <= 2)
            {
              bitDepth = state.av1c.high_bitdepth ? 10 : 8;
            }

            return bitDepth;
          };

        auto bitDepthMaster = computeBitDepth(auxImageIds.master);
        auto bitDepthAux = computeBitDepth(auxImageIds.aux);

        if(bitDepthMaster != bitDepthAux)
          out->error("An AV1 Alpha Image Item shall be encoded with the same bit depth as the associated master AV1 Image Item");
      }
    }
  },
  {
    "Section 4. Auxiliary Image Sequences\n"
    "The mono_chrome field in the Sequence Header OBU shall be set to 1.\n"
    "The color_range field in the Sequence Header OBU shall be set to 1.",
    [] (Box const& root, IReport* out)
    {
      std::vector<uint32_t> av1AlphaTrackIds;
      std::map<uint32_t /*trackId*/, int64_t> av1AplhaTrackFirstOffset;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
            {
              uint32_t trackId = 0;

              for(auto& trakChild : moovChild.children)
              {
                if(trakChild.fourcc == FOURCC("tkhd"))
                {
                  for(auto& sym : trakChild.syms)
                    if(!strcmp(sym.name, "track_ID"))
                      trackId = sym.value;
                }
                else if(trakChild.fourcc == FOURCC("mdia"))
                  for(auto& mdiaChild : trakChild.children)
                    if(mdiaChild.fourcc == FOURCC("minf"))
                      for(auto& minfChild : mdiaChild.children)
                        if(minfChild.fourcc == FOURCC("stbl"))
                          for(auto& stblChild : minfChild.children)
                          {
                            if(stblChild.fourcc == FOURCC("stsd"))
                            {
                              for(auto& stsdChild : stblChild.children)
                                if(stsdChild.fourcc == FOURCC("av01"))
                                  for(auto& sampleEntryChild : stsdChild.children)
                                    if(sampleEntryChild.fourcc == FOURCC("auxi"))
                                    {
                                      std::string auxType;

                                      for(auto& sym : sampleEntryChild.syms)
                                        if(!strcmp(sym.name, "aux_track_type"))
                                        {
                                          auxType.push_back((char)sym.value);

                                          if(auxType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")
                                            av1AlphaTrackIds.push_back(trackId);
                                        }
                                    }
                            }
                            else if(stblChild.fourcc == FOURCC("stco"))
                            {
                              if(std::find(av1AlphaTrackIds.begin(), av1AlphaTrackIds.end(), trackId) != av1AlphaTrackIds.end())
                                for(auto& sym : stblChild.syms)
                                  if(!strcmp(sym.name, "chunk_offset"))
                                    av1AplhaTrackFirstOffset[trackId] = sym.value;
                            }
                          }
              }
            }

      for(auto& offset : av1AplhaTrackFirstOffset)
      {
        auto& box = explore(root, offset.second);

        if(box.fourcc != FOURCC("mdat"))
        {
          out->error("AV1 Alpha track (ID=%u) computed offset is %lld ; located in box \"%s\" (expected \"mdat\") ; skipping", offset.first, offset.second, toString(box.fourcc).c_str());
          continue;
        }

        std::vector<uint8_t> bytes;
        int64_t diffBits = 8 * (offset.second - box.position);

        for(auto& sym : box.syms)
        {
          if(diffBits > 0)
          {
            diffBits -= sym.numBits;
            continue;
          }

          bytes.push_back((uint8_t)sym.value);

          auto const maxSeqHdrProbingLen = 128;

          if(bytes.size() >= maxSeqHdrProbingLen)
            break;
        }

        BoxReader br;
        br.br = BitReader { root.original + offset.second, (int)(box.size - (offset.second - box.position)) };

        av1State state;

        bool seqHdrFound = false;

        while(!br.empty())
        {
          auto obuType = parseAv1Obus(&br, state, false);

          if(obuType == OBU_SEQUENCE_HEADER)
          {
            seqHdrFound = true;
            break;
          }

          if(obuType == OBU_FRAME_HEADER || obuType == OBU_REDUNDANT_FRAME_HEADER || obuType == OBU_FRAME)
            /* don't parse further than our AV1 state management: if compliant we should have parsed the sequence header*/
            break;
        }

        if(!seqHdrFound)
        {
          out->error("No sequence header OBU found in 'mdat' box (track_ID=%u)", offset.first);
          return;
        }

        assert(br.myBox.children.empty());

        if(!state.av1c.mono_chrome)
          out->error("The mono_chrome field in the Sequence Header OBU shall be set to 1 (track_ID=%u)", offset.first);

        if(!state.color_range)
          out->error("The color_range field in the Sequence Header OBU shall be set to 1 (track_ID=%u)", offset.first);
      }
    }
  },
  {
    "Section 5.1\n"
    "If any of the brands defined in this document (e.g. avif or avis) is specified\n"
    "in the major_brand field of the FileTypeBox, the file extension and Internet\n"
    "Media Type should respectively be \".avif\" and \"image/avif\"",
    [] (Box const& root, IReport* out)
    {
      std::string brandMajor;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("ftyp"))
          for(auto& sym : box.syms)
            if(!strcmp(sym.name, "brand"))
              switch(sym.value)
              {
              case FOURCC("avif"): case FOURCC("avis"): case FOURCC("avio"):
                brandMajor = toString(sym.value);
                break;
              default: break;
              }

      if(brandMajor.empty())
        return;

      std::string filename;

      auto getFileExt = [&] () {
          for(auto& field : root.syms)
            if(!strcmp(field.name, "filename"))
              filename.push_back((char)field.value);

          auto extPos = filename.find_last_of('.');

          if(extPos > filename.length())
          {
            out->warning("Filename \"%s\" has no extension", filename.c_str());
            extPos = filename.length() - 1;
          }

          return filename.substr(extPos + 1);
        };

      auto const ext = getFileExt();

      if(ext != "avif")
        out->warning("file \"%s\" with major brand \"%s\" shall have extension '.avif', got '.%s'", filename.c_str(), brandMajor.c_str(), ext.c_str());
    }
  },
  {
    "Section 5.3\n"
    "If the image sequences are made only of AV1 Samples marked as sync, then the\n"
    "brand avio should be used",
    [] (Box const& root, IReport* out)
    {
      std::vector<uint32_t> av1AlphaTrackIds;
      std::map<uint32_t /*trackId*/, int64_t> av1AplhaTrackFirstOffset;

      auto isAV1 = [] (std::vector<Box> const& root) {
          for(auto& stblChild : root)
            if(stblChild.fourcc == FOURCC("stsd"))
              for(auto& stsdChild : stblChild.children)
                if(stsdChild.fourcc == FOURCC("av01"))
                  return true;

          return false;
        };

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("mdia"))
                  for(auto& mdiaChild : trakChild.children)
                    if(mdiaChild.fourcc == FOURCC("minf"))
                      for(auto& minfChild : mdiaChild.children)
                        if(minfChild.fourcc == FOURCC("stbl"))
                          if(isAV1(minfChild.children))
                            for(auto& stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stss"))
                                for(auto& sym1 : stblChild.syms)
                                  if(!strcmp(sym1.name, "sample_number"))
                                  {
                                    // check in stts (mandatory) the number of samples
                                    auto getSampleNum = [&] () {
                                        int64_t sampleNum = 0;

                                        for(auto& stblChild2 : minfChild.children)
                                          if(stblChild2.fourcc == FOURCC("stts"))
                                            for(auto& sym2 : stblChild2.syms)
                                              if(!strcmp(sym2.name, "sample_count"))
                                                sampleNum += sym2.value;

                                        return sampleNum;
                                      };

                                    auto const sampleNum = getSampleNum();

                                    if(sym1.value == sampleNum)
                                    {
                                      bool found = false;

                                      for(auto& box : root.children)
                                        if(box.fourcc == FOURCC("ftyp"))
                                          for(auto& sym : box.syms)
                                            if(!strcmp(sym.name, "compatible_brand"))
                                              if(sym.value == FOURCC("avio"))
                                                found = true;

                                      if(!found)
                                        out->warning("image sequence made only of AV1 sync samples: brand avio should be used");

                                      out->warning("\"stts\" box can be omitted since all track samples are sync");
                                    }
                                  }
    }
  },
  {
    "Section 6\n"
    "If transformative properties are used in derivation chains, they shall only be\n"
    "associated with items that are not referenced by another derived item.",
    [] (Box const& root, IReport* out)
    {
      auto isTransformative = [] (uint32_t fourcc) {
          if(fourcc == FOURCC("clap") || fourcc == FOURCC("irot") || fourcc == FOURCC("imir"))
            return true;
          else
            return false;
        };

      // locate transformative Item IDs
      // as mentioned in Section 6 ("cropping, mirroring or rotation transformations")

      std::map<uint32_t /*1-based*/, uint32_t /*fourcc*/> transformationIndices, transformationItemIDs;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipco"))
                {
                  int index = 1;

                  for(auto& ipcoChild : iprpChild.children)
                  {
                    if(isTransformative(ipcoChild.fourcc))
                      transformationIndices[index] = ipcoChild.fourcc;

                    index++;
                  }
                }

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipma"))
                {
                  uint32_t localItemId = 0;

                  for(auto& sym : iprpChild.syms)
                  {
                    if(!strcmp(sym.name, "item_ID"))
                      localItemId = sym.value;
                    else if(!strcmp(sym.name, "property_index"))
                      if(transformationIndices.find(sym.value) != transformationIndices.end())
                        transformationItemIDs.insert({ localItemId, transformationIndices[sym.value] });
                  }
                }

      // check if they are in derivation chains
      auto graph = buildDerivationGraph(root);

      auto check = [&] (const std::list<uint32_t>& visited) {
          bool foundTransformation = false;
          uint32_t lastTransformation = 0, lastTransformationItemId = 0;

          for(auto v : visited)
          {
            if(transformationItemIDs.find(v) != transformationItemIDs.end())
            {
              if(!foundTransformation)
                foundTransformation = true;
              else
                out->error("Transformative properties used in derivation chains shall only be associated\n"
                           "with items that are not referenced by another derived item.\n"
                           "However {item_ID=%u, type=%s} was preceeded by {item_ID=%u, type=%s}.",
                           v, toString(transformationItemIDs[v]).c_str(), lastTransformationItemId, toString(lastTransformation).c_str());

              lastTransformationItemId = v;
              lastTransformation = transformationItemIDs[v];
            }
          }
        };

      auto onError = [&] (const std::list<uint32_t>& visited) {
          out->error("Detected error in derivations: %s", graph.display(visited).c_str());
        };

      for(auto& c : graph.connections)
      {
        std::list<uint32_t> visited;

        if(!graph.visit(c.src, visited, onError, check))
          out->error("Detected cycle in derivations.");
      }
    }
  }
};

const std::initializer_list<RuleDesc> getRulesAvifProfiles(const SpecDesc& spec);
std::vector<RuleDesc> concatRules(const std::initializer_list<const std::initializer_list<RuleDesc>>& rules);

static const SpecDesc specAvif =
{
  "avif",
  "AVIF v1.0.0, 19 February 2019\n"
  "https://aomediacodec.github.io/av1-avif/",
  { "miaf" },
  concatRules({ rulesAvifGeneral, getRulesAvifProfiles(specAvif) }),
  getParseFunctionAvif,
};

static auto const registered = registerSpec(&specAvif);

