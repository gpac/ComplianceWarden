#include "spec.h"
#include "box_reader_impl.h"
#include <algorithm> // std::find
#include <cassert>
#include <cstring> // strcmp
#include <map>
#include <memory> // make_unique
#include <stdexcept>

void checkEssential(Box const& root, IReport* out, uint32_t fourcc);

namespace
{
int parseAv1SeqHdr(IReader* br, bool& reduced_still_picture_header)
{
  br->sym("seq_profile", 3);
  br->sym("still_picture", 1);
  reduced_still_picture_header = br->sym("reduced_still_picture_header", 1);
  br->sym("", 3);
  // incomplete parsing
  return 1;
}

int parseAv1UncompressedHeader(IReader* br, const bool reduced_still_picture_header)
{
  if(reduced_still_picture_header)
  {
    br->sym("key_frame", 0);
    br->sym("show_frame", 0);
    return 0;
  }
  else
  {
    br->sym("show_existing_frame", 1);
    br->sym("frame_type", 2);
    br->sym("show_frame", 1);
    br->sym("", 4);
    // incomplete parsing
    return 1;
  }
}

enum
{
  OBU_SEQUENCE_HEADER = 1,
  OBU_FRAME_HEADER = 3,
  OBU_METADATA = 5,
  OBU_FRAME = 6,
  OBU_REDUNDANT_FRAME_HEADER = 7,
};

void parseAv1Obus(IReader* br, bool& reduced_still_picture_header)
{
  br->sym("obu", 0); // virtual OBU separator
  br->sym("forbidden", 1);
  auto obu_type = br->sym("obu_type", 4);
  auto obu_extension_flag = br->sym("obu_extension_flag", 1);
  auto obu_has_size_field = br->sym("obu_has_size_field", 1);

  if(!obu_has_size_field)
    assert(0 && "obu_has_size_field shall be set");

  br->sym("obu_reserved_1bit", 1);

  if(obu_extension_flag)
  {
    br->sym("temporal_id", 3);
    br->sym("spatial_id", 2);
    br->sym("extension_header_reserved_3bits", 3);
  }

  auto leb128_read = [] (IReader* br, int* bytes) -> uint64_t {
      uint64_t value = 0;
      uint8_t Leb128Bytes = 0;

      for(int i = 0; i < 8; i++)
      {
        uint8_t leb128_byte = br->sym("leb128_byte", 8);
        value |= (((uint64_t)(leb128_byte & 0x7f)) << (i * 7));
        Leb128Bytes += 1;

        if(!(leb128_byte & 0x80))
          break;
      }

      if(bytes)
        *bytes = Leb128Bytes;

      return value;
    };

  int leb128Bytes = 0;
  auto obuSize = leb128_read(br, &leb128Bytes);
  switch(obu_type)
  {
  case OBU_SEQUENCE_HEADER:
    br->sym("seqhdr", 0);
    obuSize -= parseAv1SeqHdr(br, reduced_still_picture_header);
    break;
  case OBU_FRAME_HEADER: case OBU_REDUNDANT_FRAME_HEADER: case OBU_FRAME:
    br->sym("frmhdr", 0);
    obuSize -= parseAv1UncompressedHeader(br, reduced_still_picture_header);
    break;
  case OBU_METADATA:
    br->sym("meta", 0);
    break;
  default: break;
  }

  while(obuSize-- > 0)
    br->sym("byte", 8);
}

void parseAv1C(IReader* br)
{
  br->sym("marker", 1);
  br->sym("version", 7);
  br->sym("seq_profile", 3);
  br->sym("seq_level_idx_0", 5);
  br->sym("seq_tier_0", 1);
  br->sym("high_bitdepth", 1);
  br->sym("twelve_bit", 1);
  br->sym("monochrome", 1);
  br->sym("chroma_subsampling_x", 1);
  br->sym("chroma_subsampling_y", 1);
  br->sym("chroma_sample_position", 2);
  br->sym("reserved", 3);

  auto initial_presentation_delay_present = br->sym("initial_presentation_delay_present", 1);

  if(initial_presentation_delay_present)
  {
    br->sym("initial_presentation_delay_minus_one", 4);
  }
  else
  {
    br->sym("reserved", 4);
  }

  br->sym("configOBUs", 0);

  while(!br->empty())
  {
    bool reduced_still_picture_header = false;
    parseAv1Obus(br, reduced_still_picture_header);
  }
}

ParseBoxFunc* getParseFunctionAvif(uint32_t fourcc)
{
  switch(fourcc)
  {
  case FOURCC("av1C"):
    return &parseAv1C;
  default:
    return nullptr;
  }
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

struct ItemLocation
{
  int construction_method = 0, data_reference_index = 0;
  int64_t base_offset = 0;
  struct Span
  {
    int64_t offset = 0, length = 0;
  };
  std::vector<Span> extents;

  int computeOffset(IReport* out)
  {
    if(construction_method > 1 || extents.size() > 1)
    {
      out->warning("construction_method > 1 not supported");
      return 0;
    }

    if(extents.size() > 1)
    {
      out->warning("iloc with several extentions not supported");
      return 0;
    }

    if(data_reference_index > 0)
    {
      out->warning("data_reference_index > 0 not supported");
      return 0;
    }

    int originOffset = base_offset;

    if(!extents.empty())
      originOffset += extents[0].offset;

    return originOffset;
  }
};

std::vector<ItemLocation::Span> getAv1ImageItemsDataOffsets(Box const& root, IReport* out, uint32_t itemIDs)
{
  std::vector<ItemLocation::Span> spans;

  std::vector<ItemLocation> itemLocs;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iloc"))
        {
          int64_t currOffset = 0;

          for(auto& sym : metaChild.syms)
          {
            if(!strcmp(sym.name, "item_ID"))
            {
              if(sym.value != itemIDs)
                break;

              itemLocs.resize(itemLocs.size() + 1);

              continue;
            }

            if(itemLocs.empty())
              continue;

            auto& itemLoc = itemLocs.back();

            if(!strcmp(sym.name, "construction_method"))
              itemLoc.construction_method = sym.value;

            if(!strcmp(sym.name, "data_reference_index"))
              itemLoc.data_reference_index = sym.value;

            if(!strcmp(sym.name, "base_offset"))
              itemLoc.base_offset = sym.value;

            if(!strcmp(sym.name, "extent_offset"))
              currOffset = sym.value;

            if(!strcmp(sym.name, "extent_length"))
              itemLoc.extents.push_back({ currOffset, sym.value });
          }
        }

  for(auto& itemLoc : itemLocs)
    spans.push_back({ itemLoc.computeOffset(out), itemLoc.extents[0].length }); // we assume no idat-based check (construction_method = 1)

  return spans;
}

Box const& explore(Box const& root, uint64_t targetOffset)
{
  for(auto& box : root.children)
    if(box.position + box.size > targetOffset)
      return explore(box, targetOffset);

  return root;
}

std::vector<uint8_t> getAV1ImageItemData(Box const& root, IReport* out, uint32_t itemId)
{
  std::vector<uint8_t> bytes;
  auto spans = getAv1ImageItemsDataOffsets(root, out, itemId);

  for(auto span : spans)
  {
    auto box = explore(root, span.offset);
    int64_t diffBits = 8 * (span.offset - box.position);

    bool firstTime = true;

    for(auto sym : box.syms)
    {
      if(diffBits > 0)
      {
        diffBits -= sym.numBits;
        continue;
      }

      if(firstTime)
      {
        if(diffBits && diffBits + sym.numBits)
          out->warning("Could not locate AV1 Image Item Data (diffBits=%s)", toString(diffBits).c_str());

        firstTime = false;
      }

      if(strlen(sym.name) || sym.numBits != 8)
      {
        out->warning("Wrong symbol detected (name=%s, size=%d bits). Stop import.", sym.name, sym.numBits);
        break;
      }

      bytes.push_back((uint8_t)sym.value);

      if((int64_t)bytes.size() >= span.length)
        break;
    }
  }

  return bytes;
}
} // namespace

static const SpecDesc spec =
{
  "avif",
  "AVIF v1.0.0, 19 February 2019\n"
  "https://aomediacodec.github.io/av1-avif/",
  { "heif" },
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
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          bool reduced_still_picture_header = false;

          while(!br.empty())
            parseAv1Obus(&br, reduced_still_picture_header);

          /* we know we've seen the sequence header (av1C) and the frame header*/
          bool showFrame = false, keyFrame = false;
          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
          {
            if(!strcmp(sym.name, "show_frame"))
              showFrame = true;

            if(!strcmp(sym.name, "key_frame"))
              keyFrame = true;
          }

          if(!(showFrame && keyFrame))
            out->error("AV1 Sample shall be marked as sync");
        }
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data shall have exactly one Sequence Header OBU.",
      [] (Box const& root, IReport* out)
      {
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        int seqHdrNum = 0;

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          bool reduced_still_picture_header = false;

          while(!br.empty())
            parseAv1Obus(&br, reduced_still_picture_header);

          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
            if(!strcmp(sym.name, "seqhdr"))
              seqHdrNum++;
        }

        if(seqHdrNum != 1)
          out->error("Expected 1 sequence Header OBU but found %s", seqHdrNum);
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
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          bool reduced_still_picture_header = false;

          while(!br.empty())
            parseAv1Obus(&br, reduced_still_picture_header);

          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
            if(!strcmp(sym.name, "still_picture"))
              if(sym.value == 0)
                out->error("still_picture flag set to 0.");
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
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          bool reduced_still_picture_header = false;

          while(!br.empty())
            parseAv1Obus(&br, reduced_still_picture_header);

          if(!reduced_still_picture_header)
            out->error("reduced_still_picture_header flag set to 0.");
        }
      }
    },
    {
      "Section 2.2.1\n"
      "If a Sequence Header OBU is present in the AV1CodecConfigurationBox, it shall match the\n"
      "Sequence Header OBU in the AV1 Image Item Data.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "The values of the fields in the AV1CodecConfigurationBox shall match those of the\n"
      "Sequence Header OBU in the AV1 Image Item Data.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
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
#if 0
    {
      "Section 4. Auxiliary Image Items\n"
      "The mono_chrome field in the Sequence Header OBU shall be set to 1.",
      [] (Box const& root, IReport* out)
      {
        // TODO: is an AV1 Image Item OR  AV1 Image Sequence => Check against 2.1
        // TODO: check Auxiliary

        // TODO: parse seq hdr until color config
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          bool reduced_still_picture_header = false;

          while(!br.empty())
            parseAv1Obus(&br, reduced_still_picture_header);

          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
            if(!strcmp(sym.name, "still_picture"))
              if(sym.value == 0)
                out->error("still_picture flag set to 0.");
        }
      }
    },
#endif
  },
  getParseFunctionAvif,
};

static auto const registered = registerSpec(&spec);

