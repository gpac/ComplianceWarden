#include <cstring>
#include "spec.h"
#include "fourcc.h"
#include "bit_reader.h"
#include "isobmff_derivations.h"
#include <algorithm> // std::find

extern std::vector<uint32_t> visualSampleEntryFourccs;
bool isVisualSampleEntry(uint32_t fourcc);
std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);
void checkEssential(Box const& root, IReport* out, uint32_t fourcc);
std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> getItemDataOffsets(Box const& root, IReport* out, uint32_t itemID);
void boxCheck(Box const& root, IReport* out, std::vector<uint32_t> oneOf4CCs, std::vector<uint32_t> parent4CCs, std::pair<unsigned, unsigned> expectedAritySpan);
bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root);

namespace
{
bool visitDerivationsBackward(DerivationGraph& graph, uint32_t itemIdDst, std::list<uint32_t>& visited, std::function<void(const std::list<uint32_t> &)> onError)
{
  auto const maxDerivations = 16;
  visited.push_back(itemIdDst);

  for(auto& c : graph.connections)
  {
    if(c.dst == itemIdDst)
    {
      if(c.dst == c.src || visited.size() > maxDerivations // cycles
         || !visitDerivationsBackward(graph, c.src, visited, onError))
      {
        visited.push_back(c.src);
        onError(visited);
        return false;
      }
    }
  }

  return true;
}

void checkDerivation(Box const& root, IReport* out, uint32_t fourcc, std::function<void(uint32_t, std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>>)> check)
{
  std::vector<uint32_t> itemIds;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iinf"))
          for(auto& iinfChild : metaChild.children)
            if(iinfChild.fourcc == FOURCC("infe"))
            {
              uint32_t itemId = 0;

              for(auto& sym : iinfChild.syms)
                if(!strcmp(sym.name, "item_ID"))
                  itemId = sym.value;
                else if(!strcmp(sym.name, "item_type"))
                  if(sym.value == fourcc)
                    itemIds.push_back(itemId);
            }

  for(auto& itemId : itemIds)
  {
    auto const spans = getItemDataOffsets(root, out, itemId);

    if(spans.empty())
    {
      out->error("No data offset found for item ID %u", itemId);
      continue;
    }

    for(auto& span : spans)
    {
      if(span.first + span.second > (int64_t)root.size)
      {
        out->error("Image data (itemID=%u): found offset %lld + size %lld while file size is only %llu\n",
                   itemId, span.first, span.second, root.size);
        continue;
      }
    }

    for(auto& span : spans)
    {
      if(span.second == 0)
      {
        out->error("Image data (itemID=%u): found invalid span size %lld\n", itemId, span.second);
        continue;
      }
    }

    check(itemId, spans);
  }
}

void checkDerivationVersion(Box const& root, IReport* out, uint32_t fourcc)
{
  auto check = [&] (uint32_t itemId, std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> spans) {
    if(spans[0].second == 0)
    {
      out->error("Image data (itemID=%u): found invalid span size %lld\n", itemId, spans[0].second);
      return;
    }

    auto br = BitReader { root.original + spans[0].first, (int)spans[0].second };
    auto version = br.u(8);

    if(version != 0)
      out->error("'%s' version shall be equal to 0, found %lld (itemId=%u)", toString(fourcc).c_str(), version, itemId);
  };

  checkDerivation(root, out, fourcc, check);
}
}

static const SpecDesc specHeif =
{
  "heif",
  "HEIF - ISO/IEC 23008-12 - 2nd Edition N18310",
  { "isobmff" },
  {
    {
      "Section 10.2.1.1\n"
      "Files shall contain the brand 'mif1' in the compatible brands array of the\n"
      "FileTypeBox.\n"
      "/!\\ this rule doesn't look for 'mif1' brands rule-conformance:\n"
      "     if a brand is absent then its conformance rules won't be checked here /!\\",
      [] (Box const& root, IReport* out)
      {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
        {
          out->error("'ftyp' box not found");
          return;
        }

        auto& ftypBox = root.children[0];

        bool found = false;

        for(auto& brand : ftypBox.syms)
          if(!strcmp(brand.name, "compatible_brand") && brand.value == FOURCC("mif1"))
            found = true;

        if(!found)
          out->error("'mif1' brand not found in 'ftyp' box");
      }
    },
    {
      "Section 7.5.3.1\n"
      "The nature of the auxiliary track is announced by the AuxiliaryTypeInfoBox that\n"
      "shall be included in the sample entry of the auxiliary track.",
      [] (Box const& root, IReport* out)
      {
        auto findTrackId = [] (Box const& root) -> uint32_t {
          for(auto& trakChild : root.children)
            if(trakChild.fourcc == FOURCC("tkhd"))
              for(auto& sym : trakChild.syms)
                if(!strcmp(sym.name, "track_ID"))
                  return (uint32_t)sym.value;

          return 0;
        };

        struct Track
        {
          uint32_t videoTrackId = 0, auxlTrackId = 0;
        };

        auto findAuxlTracks = [findTrackId] (Box const& root, IReport* out) -> std::vector<Track> {
          std::vector<Track> trackIds;

          for(auto& box : root.children)
            if(box.fourcc == FOURCC("moov"))
              for(auto& moovChild : box.children)
                if(moovChild.fourcc == FOURCC("trak"))
                {
                  // find the hldr
                  uint32_t handlerType = 0;

                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("mdia"))
                      for(auto& mdiaChild : trakChild.children)
                        if(mdiaChild.fourcc == FOURCC("hdlr"))
                          for(auto& sym : mdiaChild.syms)
                            if(!strcmp(sym.name, "handler_type"))
                              handlerType = (uint32_t)sym.value;

                  // find tref track_id
                  uint32_t trackId = 0;

                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("tref"))
                      for(auto& trefChild : trakChild.children)
                        if(trefChild.fourcc == FOURCC("auxl"))
                          for(auto& sym : trefChild.syms)
                            if(!strcmp(sym.name, "track_IDs"))
                            {
                              if(trackId)
                                out->error("Unexpected: found several 'tref' track_IDs (%u then %lld)", trackId, sym.value);

                              trackId = (uint32_t)sym.value;
                            }

                  if(handlerType == FOURCC("pict") && trackId)
                  {
                    auto id = findTrackId(moovChild);

                    if(id)
                      trackIds.push_back({ trackId, id });
                  }
                }

          return trackIds;
        };

        auto auxlTracks = findAuxlTracks(root, out);

        auto findAuxlVideoTrack = [&] (uint32_t trackId) -> uint32_t {
          for(auto& auxl : auxlTracks)
            if(auxl.auxlTrackId == trackId)
              return auxl.videoTrackId;

          return 0;
        };

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak"))
              {
                auto const trackIdAuxl = findTrackId(moovChild);
                auto const trackIdVideo = findAuxlVideoTrack(trackIdAuxl);

                if(!trackIdVideo)
                  continue;

                bool foundAuxi = false;

                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto& mdiaChild : trakChild.children)
                    {
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto& minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl"))
                            for(auto& stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto& stsdChild : stblChild.children)
                                  if(isVisualSampleEntry(stsdChild.fourcc))
                                    for(auto& sampleEntryChild : stsdChild.children)
                                      if(sampleEntryChild.fourcc == FOURCC("auxi"))
                                      {
                                        if(!foundAuxi)
                                          foundAuxi = true;
                                        else
                                          out->error("AuxiliaryTypeInfoBox ('auxi') is present several times (trackIDs: video=%u, aux=%u)", trackIdVideo, trackIdAuxl);
                                      }
                    }

                if(!foundAuxi)
                  out->error("AuxiliaryTypeInfoBox ('auxi') is absent (trackIDs: video=%u, aux=%u)", trackIdVideo, trackIdAuxl);
              }
      }
    },
    {
      "Section 6.2\n"
      "A MetaBox ('meta'), as specified in ISO/IEC 14496-12, is required at file level.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            found = true;

        if(!found)
          out->error("'meta' box not found at file level");
      }
    },
    {
      "Section 6.2\n"
      "The handler type for the MetaBox shall be 'pict'.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("hdlr"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "handler_type"))
                  {
                    found = true;

                    if(field.value != FOURCC("pict"))
                      out->error("The handler type for the MetaBox shall be 'pict'");
                  }

        if(!found)
          out->error("'hdlr' not found in MetaBox");
      }
    },
    {
      "Section 6.6.2.1\n"
      "A derived image item of the item_type value 'iden' (identity transformation)\n"
      "may be used when it is desired to use transformative properties to derive\n"
      "an image item. The derived image item shall have no item body (i.e. no extents)\n"
      "and reference_count for the 'dimg' item  reference of a 'iden' derived image\n"
      "item shall be equal to 1.",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> idenItemIds;

        // find iden items
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
                        if(sym.value == FOURCC("iden"))
                          idenItemIds.push_back(itemId);
                    }
                  }

        // the derived image item shall have no item body (i.e. no extents)
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iloc"))
              {
                // for MIAF coded image items construction_method==0 and data_reference_index==0
                uint32_t itemId = 0;

                for(auto& sym : metaChild.syms)
                {
                  if(!strcmp(sym.name, "item_ID"))
                    itemId = sym.value;

                  if(std::find(idenItemIds.begin(), idenItemIds.end(), sym.value) != idenItemIds.end())
                    if(!strcmp(sym.name, "extent_count"))
                      if(sym.value != 0)
                        out->error("The derived image item [ID=%u] shall have no item body (i.e. no extents), found %lld", itemId, sym.value);
                }
              }

        // reference_count for the 'dimg' item reference of a 'iden' derived image item shall be equal to 1
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iref"))
              {
                uint32_t boxType = -1, itemId = 0;

                for(auto& sym : metaChild.syms)
                {
                  if(!strcmp(sym.name, "box_type"))
                    boxType = sym.value;

                  if(boxType != FOURCC("dimg"))
                    continue;

                  if(!strcmp(sym.name, "from_item_ID"))
                  {
                    if(std::find(idenItemIds.begin(), idenItemIds.end(), sym.value) == idenItemIds.end())
                      boxType = -1; // abort

                    itemId = sym.value;
                  }

                  if(!strcmp(sym.name, "reference_count"))
                    if(sym.value != 1)
                      out->error("reference_count for the 'dimg' item reference of a 'iden' derived image item (ID=%u) "
                                 "shall be equal to 1: found %lld", itemId, sym.value);
                }
              }
      }
    },
    {
      "Section 6.6.2.3.1\n"
      "[tiles] the value of reference_count shall be equal to rows*columns",
      [] (Box const& root, IReport* out)
      {
        // reference_count
        std::map<uint32_t /*itemId*/, int> refCounts;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iref"))
              {
                uint32_t boxType = -1, itemId = 0;

                for(auto& sym : metaChild.syms)
                {
                  if(!strcmp(sym.name, "box_type"))
                    boxType = sym.value;

                  if(boxType != FOURCC("dimg"))
                    continue;

                  if(!strcmp(sym.name, "from_item_ID"))
                    itemId = sym.value;

                  if(!strcmp(sym.name, "reference_count"))
                    refCounts[itemId] = sym.value;
                }
              }

        auto check = [&] (uint32_t itemId, std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> spans) {
          if(spans[0].second == 0)
          {
            out->error("[tile] Image data (itemID=%u): found invalid span size %lld", itemId, spans[0].second);
            return;
          }

          if(spans.size() > 1)
            out->error("[tile] ItemID=%u: multiple spans (%d) not handled. Only considering the first one.", itemId, (int)spans.size());

          if(spans[0].second < 2)
          {
            out->error("[tile] ItemID=%u: not enough bytes to parse: %d instead of 2 to compute FieldLength.", itemId, spans[0].second);
            return;
          }

          auto br = BitReader { root.original + spans[0].first, (int)spans[0].second };
          br.u(8); /*version*/
          auto const flags = br.u(8);
          const int fieldLength = ((flags & 1) + 1) * 2;

          if(spans[0].second < 4 + fieldLength * 2)
          {
            out->error("[tile] ItemID=%u: not enough bytes to parse: %d instead of %d.", itemId, spans[0].second, 4 + fieldLength * 2);
            return;
          }

          auto const rows = 1 + br.u(8);
          auto const columns = 1 + br.u(8);

          if(rows * columns != refCounts[itemId])
            out->error("Tile [itemId=%u]: the value of reference_count(%d) shall be equal to rows(%d)*columns(%d)=%d",
                       itemId, refCounts[itemId], rows, columns, rows * columns);
        };

        checkDerivation(root, out, FOURCC("grid"), check);
      }
    },
    {
      "Section 6.6.2.3.1\n"
      "Tiles: the values of to_item_ID [shall] identify the input images",
      [] (Box const& root, IReport* out)
      {
        std::map<uint32_t /*itemID*/, uint32_t /*fourcc*/> itemFourccs;

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
                        itemFourccs.insert({ itemId, sym.value });
                    }
                  }

        auto d = getDerivationsInfo(root, FOURCC("dimg"));

        for(auto& item : d.itemRefs)
          for(auto toItemId : item.second)
          {
            // we need to check the derivation graph in case some 'iref's are missing
            auto graph = buildDerivationGraph(root);

            auto onError = [&] (const std::list<uint32_t>& visited) {
              out->error("Detected error in derivations: %s", graph.display(visited).c_str());
            };

            std::list<uint32_t> visitedBackward;

            if(!visitDerivationsBackward(graph, toItemId, visitedBackward, onError))
            {
              out->error("Tiles: to_item_ID=%u derivation chain is cyclic", toItemId);
            }
            else
            {
              auto finalItemId = visitedBackward.back();

              if(!isVisualSampleEntry(itemFourccs[finalItemId]))
                out->error("Tiles: coded image (ItemID=%u derived from ItemID=%u) has type \"%s\" which doesn't seem to identify an input image. "
                           "Backward derivation graph from ItemID=%u: %s",
                           toItemId, finalItemId, toString(itemFourccs[finalItemId]).c_str(), toItemId, graph.display(visitedBackward).c_str());
            }
          }
      }
    },
    {
      "Section 6.6.2.3.1\n"
      "Tiles: all input images shall have exactly the same width and height",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> gridItemIds;

        // find 'grid' items

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
                        if(sym.value == FOURCC("grid"))
                          gridItemIds.push_back(itemId);
                    }
                  }

        // check ispe for each grid item
        auto d = getDerivationsInfo(root, FOURCC("dimg"));

        for(auto gridItemId : gridItemIds)
        {
          Resolution resIspe;

          for(auto& iref : d.itemRefs)
            if(iref.first == gridItemId)
              for(auto itemId : iref.second)
              {
                if(resIspe.width == Resolution().width)
                  resIspe = d.itemRes[itemId];
                else if(d.itemRes[itemId].width != resIspe.width || d.itemRes[itemId].height != resIspe.height)
                  out->error("Tiles [ItemId]: all input images shall have exactly the same width and height\n"
                             "but found %dx%d for itemID=%u in 'ispe'",
                             gridItemId, resIspe.width, resIspe.height, d.itemRes[itemId].width, d.itemRes[itemId].height, itemId);
              }
        }
      }
    },
    {
      "Section 6.6.2.3.1: tiles\n"
      "The tiled input images shall completely “cover” the reconstructed image grid\n"
      "canvas, where tile_width*columns is greater than or equal to output_width and\n"
      "tile_height*rows is greater than or equal to output_height.",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> gridItemIds;

        // find 'grid' items

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
                        if(sym.value == FOURCC("grid"))
                          gridItemIds.push_back(itemId);
                    }
                  }

        // find grids resolutions and layouts
        std::map<uint32_t /*ItemID*/, Resolution> gridResolutions;
        std::map<uint32_t /*ItemID*/, std::pair<int /*numRows*/, int /*numCols*/>> gridLayouts;

        auto check = [&] (uint32_t itemId, std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> spans) {
          if(spans[0].second == 0)
          {
            out->error("[grid] Image data (itemID=%u): found invalid span size %lld", itemId, spans[0].second);
            return;
          }

          if(spans.size() > 1)
            out->error("[grid] ItemID=%u: multiple spans (%d) not handled. Only considering the first one.", itemId, (int)spans.size());

          if(spans[0].second < 2)
          {
            out->error("[grid] ItemID=%u: not enough bytes to parse: %d instead of 2 to compute FieldLength.", itemId, spans[0].second);
            return;
          }

          auto br = BitReader { root.original + spans[0].first, (int)spans[0].second };
          br.u(8); /*version*/
          auto const flags = br.u(8);
          const int fieldLength = ((flags & 1) + 1) * 2;

          if(spans[0].second < 4 + fieldLength * 2)
          {
            out->error("[grid] ItemID=%u: not enough bytes to parse: %d instead of %d.", itemId, spans[0].second, 4 + fieldLength * 2);
            return;
          }

          auto const rows_minus_one = br.u(8);
          auto const columns_minus_one = br.u(8);
          const int output_width = br.u(fieldLength * 8);
          const int output_height = br.u(fieldLength * 8);

          gridResolutions[itemId] = Resolution { output_width, output_height };
          gridLayouts[itemId] = { rows_minus_one + 1, columns_minus_one + 1 };
        };

        checkDerivation(root, out, FOURCC("grid"), check);

        // ensure each grid item is covered: we know from previous tests that sizes are constant and consistent
        auto d = getDerivationsInfo(root, FOURCC("dimg"));

        for(auto gridItemId : gridItemIds)
          for(auto& iref : d.itemRefs)
            if(iref.first == gridItemId)
            {
              for(auto itemId : iref.second)
                if(d.itemRes[itemId].width * gridLayouts[gridItemId].second < gridResolutions[gridItemId].width)
                  out->error("grid (itemID=%u) width(%d) not covered by tile (ItemId=%u) width(%d)*numColumns(%d)=%d",
                             gridItemId, itemId, gridResolutions[gridItemId].width,
                             d.itemRes[itemId].width, gridLayouts[gridItemId].second, d.itemRes[itemId].width * gridLayouts[gridItemId].second);
                else if(d.itemRes[itemId].height * gridLayouts[gridItemId].first < gridResolutions[gridItemId].height)
                  out->error("grid (itemID=%u) height(%d) not covered by tile (ItemId=%u) height(%d)*numRows(%d)=%d",
                             gridItemId, itemId, gridResolutions[gridItemId].height,
                             d.itemRes[itemId].height, gridLayouts[gridItemId].first, d.itemRes[itemId].height * gridLayouts[gridItemId].first);
            }
      }
    },
    {
      "Section 7.2.1\n"
      "The following constraints on the value of matrix [of the TrackHeaderBox] shall be obeyed",
      [] (Box const& root, IReport* out)
      {
        std::vector<int64_t> matrix; // a, b, u, c, d, v, x, y, w

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak"))
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd"))
                    for(auto& sym : trakChild.syms)
                      if(!strcmp(sym.name, "matrix"))
                        matrix.push_back(sym.value);

        if(!matrix.empty())
        {
          // either a or c but not both shall be equal to 0x00010000 or 0xFFFF0000, while the remaining one of a and c shall be equal to 0;
          if(matrix[0])
          {
            if(matrix[0] != 0x00010000 && matrix[0] != 0xffff0000)
              out->error("Matrix field of the TrackHeaderBox: \"a\" value (%llX) shall be 0x00010000 or 0xFFFF0000", matrix[0]);

            if(matrix[3])
              out->error("Matrix field of the TrackHeaderBox: \"a\" (%llX) or \"c\" (%llX) shall be equal to 0", matrix[0], matrix[3]);
          }
          else if(matrix[3] != 0x00010000 && matrix[3] != 0xffff0000)
            out->error("Matrix field of the TrackHeaderBox: \"c\" value (%llX) shall be 0x00010000 or 0xFFFF0000 (\"a\" is 0)", matrix[3]);

          // either b or d but not both shall be equal to 0x00010000 or 0xFFFF0000, while the remaining one of b and d shall be equal to 0;
          if(matrix[1])
          {
            if(matrix[1] != 0x00010000 && matrix[1] != 0xffff0000)
              out->error("Matrix field of the TrackHeaderBox: \"b\" value (%llX) shall be 0x00010000 or 0xFFFF0000", matrix[1]);

            if(matrix[4])
              out->error("Matrix field of the TrackHeaderBox: \"b\" (%llX) or \"d\" (%llX) shall be equal to 0", matrix[1], matrix[4]);
          }
          else if(matrix[4] != 0x00010000 && matrix[4] != 0xffff0000)
            out->error("Matrix field of the TrackHeaderBox: \"d\" value (%llX) shall be 0x00010000 or 0xFFFF0000 (\"b\" is 0)", matrix[5]);

          if(matrix[2])
            out->error("Matrix field of the TrackHeaderBox: \"u\" value (%llX) shall be 0", matrix[2]);

          if(matrix[5])
            out->error("Matrix field of the TrackHeaderBox: \"v\" value (%llX) shall be 0", matrix[5]);

          if(matrix[8] != 0x40000000)
            out->error("Matrix field of the TrackHeaderBox: \"w\" value (%llX) shall be 0x40000000", matrix[8]);
        }
      }
    },
    {
      "Section 7.2.1.9\n"
      "ItemInfoBox "
      "Version 0 or 1 of this box is required by ISO/IEC 23008-12",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iinf"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "version"))
                  {
                    if(field.value == 0 || field.value == 1)
                      found = true;
                    else
                      out->error("Version 0 or 1 of ItemInfoBox is required");
                  }

        if(!found)
          out->error("ItemInfoBox is required");
      },
    },
    {
      "Section 6.5.3.1\n"
      "Every image item shall be associated with one [image spatial extents property],\n"
      "prior to the association of all transformative properties.",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t /*4cc*/> properties;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipco"))
                    for(auto& ipcoChild : iprpChild.children)
                      properties.push_back(ipcoChild.fourcc);

        if(std::find(properties.begin(), properties.end(), FOURCC("ispe")) == properties.end())
          out->error("image spatial extents property (\"ispe\") not detected.");

        auto isTransformative = [] (uint32_t fourcc) {
          if(fourcc == FOURCC("clap") || fourcc == FOURCC("irot") || fourcc == FOURCC("imir"))
            return true;
          else
            return false;
        };

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    uint32_t localItemId = 0;
                    bool foundIspe = true;
                    auto checkIspe = [&] () {
                      if(!foundIspe)
                        out->error("Item ID=%u: missing Image spatial extents property", localItemId);
                    };

                    for(auto& sym : iprpChild.syms)
                      if(!strcmp(sym.name, "item_ID"))
                      {
                        checkIspe();
                        foundIspe = false;
                        localItemId = sym.value;
                      }
                      else if(!strcmp(sym.name, "property_index"))
                      {
                        if(sym.value /*1-based*/ > (int)properties.size())
                        {
                          out->error("Invalid property_index=%lld", sym.value);
                        }
                        else
                        {
                          if(properties[sym.value - 1] == FOURCC("ispe"))
                            foundIspe = true;
                          else if(isTransformative(properties[sym.value - 1]) && !foundIspe) // Romain => ON NE CHECKE PAS SI C'EST UNE IMAGE!!!!
                            out->error("Item ID=%u: transformative property \"%s\" (index=%lld) found prior to \"ispe\"",
                                       localItemId, toString(properties[sym.value - 1]).c_str(), sym.value);
                        }
                      }

                    checkIspe();
                  }
      },
    },
    {
      "Section 7.2.3.1\n"
      "The CodingConstraintsBox shall be present in the sample description entry for\n"
      "tracks with handler_type equal to 'pict'",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak"))
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto& mdiaChild : trakChild.children)
                    {
                      auto isPict = [&] () {
                        for(auto& mdiaChild2 : trakChild.children)
                          if(mdiaChild2.fourcc == FOURCC("hdlr"))
                            for(auto& sym : mdiaChild2.syms)
                              if(!strcmp(sym.name, "handler_type"))
                                if(sym.value == FOURCC("pict"))
                                  return true;

                        return false;
                      };

                      if(!isPict())
                        continue;

                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto& minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl"))
                            for(auto& stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto& stsdChild : stblChild.children)
                                {
                                  if(isVisualSampleEntry(stsdChild.fourcc))
                                  {
                                    bool foundCcst = false;

                                    for(auto& sampleEntryChild : stsdChild.children)
                                      if(sampleEntryChild.fourcc == FOURCC("ccst"))
                                      {
                                        if(!foundCcst)
                                          foundCcst = true;
                                        else
                                          out->error("CodingConstraintsBox ('ccst') is present several times");
                                      }

                                    if(!foundCcst)
                                      out->error("CodingConstraintsBox ('ccst') shall be present once");
                                  }
                                }
                    }
      },
    },
    {
      "Section 7.2.3.1\n"
      "The CodingConstraintsBox reserved field shall be equal to 0 in files conforming\n"
      "to this version of this specification",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak"))
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto& mdiaChild : trakChild.children)
                    {
                      auto isPict = [&] () {
                        for(auto& mdiaChild2 : trakChild.children)
                          if(mdiaChild2.fourcc == FOURCC("hdlr"))
                            for(auto& sym : mdiaChild2.syms)
                              if(!strcmp(sym.name, "handler_type"))
                                if(sym.value == FOURCC("pict"))
                                  return true;

                        return false;
                      };

                      if(!isPict())
                        continue;

                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto& minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl"))
                            for(auto& stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto& stsdChild : stblChild.children)
                                  if(isVisualSampleEntry(stsdChild.fourcc))
                                    for(auto& sampleEntryChild : stsdChild.children)
                                      if(sampleEntryChild.fourcc == FOURCC("ccst"))
                                      {
                                        auto sampleEntryChildSizeInBits = 0;

                                        for(auto sym : sampleEntryChild.syms)
                                          sampleEntryChildSizeInBits += sym.numBits;

                                        sampleEntryChildSizeInBits += sampleEntryChildSizeInBits % 8 ? 8 - sampleEntryChildSizeInBits % 8 : 0;

                                        if(sampleEntryChildSizeInBits / 8 != 16)
                                          out->error("invalid 'ccst' size: %llu instead of 16", sampleEntryChild.size);

                                        for(auto sym : sampleEntryChild.syms)
                                          if(!strcmp(sym.name, "reserved"))
                                            if(sym.value != 0)
                                              out->error("invalid 'ccst' reserved value shall be 0, found 0x%llX", sym.value);
                                      }
                    }
      }
    },
    {
      "Section 6.2\n"
      "The file-level MetaBox shall identify as its primary item an item that is a\n"
      "coded image or a derived image item",
      [] (Box const& root, IReport* out)
      {
        uint32_t primaryItemId = -1;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("pitm"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "item_ID"))
                    primaryItemId = field.value;

        if(primaryItemId == (uint32_t)-1)
        {
          out->error("No primary item.");
          return;
        }

        bool found = false;

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
                        if(itemId == primaryItemId)
                        {
                          if(found)
                            out->error("primary item (Item_ID=%u) has several associated item_types.", primaryItemId);

                          if(!isVisualSampleEntry(sym.value) // coded item
                             && sym.value != FOURCC("iden") && sym.value != FOURCC("grid") && sym.value != FOURCC("iovl")) // derivation
                            out->error("primary item (Item_ID=%u) is not coded image or a derived image item (found item_type=\"%s\")",
                                       itemId, toString(sym.value).c_str());

                          found = true;
                        }
                    }
                  }

        if(!found)
          out->error("primary item (Item_ID=%u) has no associated item_type.", primaryItemId);
      }
    },
    {
      "Section 6.4.2\n"
      "The primary item shall not be a hidden image item",
      [] (Box const& root, IReport* out)
      {
        uint32_t primaryItemId = -1;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("pitm"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "item_ID"))
                    primaryItemId = field.value;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iinf"))
                for(auto& iinfChild : metaChild.children)
                  if(iinfChild.fourcc == FOURCC("infe"))
                    for(auto& sym : iinfChild.syms)
                    {
                      if(!strcmp(sym.name, "flags"))
                        if(!(sym.value & 1))
                          // ISOBMFF 8.11.6.1: (flags & 1) equal to 1 indicates that the item is not intended to be a part of the presentation
                          break;

                      if(!strcmp(sym.name, "item_ID"))
                        if(sym.value == primaryItemId)
                          out->error("The primary item shall not be a hidden image item");
                    }
      },
    },
    {
      "Section 6.4.4\n"
      "A thumbnail image shall not be linked to another thumbnail image with\n"
      "the 'thmb' item reference.",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> thmbIds;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iref"))
              {
                uint32_t boxType = -1;

                for(auto& field : metaChild.syms)
                {
                  if(!strcmp(field.name, "box_type"))
                    boxType = field.value;

                  if(boxType != FOURCC("thmb"))
                    continue;

                  if(!strcmp(field.name, "to_item_ID"))
                    thmbIds.push_back(field.value);
                }

                uint32_t fromId = -1;

                for(auto& field : metaChild.syms)
                {
                  if(!strcmp(field.name, "box_type"))
                    boxType = field.value;

                  if(boxType != FOURCC("thmb"))
                    continue;

                  if(!strcmp(field.name, "from_item_ID"))
                    fromId = field.value;

                  if(!strcmp(field.name, "to_item_ID"))
                    if(std::find(thmbIds.begin(), thmbIds.end(), fromId) != thmbIds.end())
                      out->error("Thumbnail image is linked to another thumbnail image (from item_ID=%u to item_ID=%u)", fromId, field.value);
                }
              }
      }
    },
    {
      "Section 6.4\n"
      "The number of SingleItemTypeReferenceBoxes with the box type 'dimg' and with the\n"
      "same value of from_item_ID shall not be greater than 1",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iref"))
              {
                uint32_t boxType = -1;

                std::vector<uint32_t> fromIds;

                for(auto& field : metaChild.syms)
                {
                  if(!strcmp(field.name, "box_type"))
                    boxType = field.value;

                  if(boxType != FOURCC("dimg"))
                    continue;

                  if(!strcmp(field.name, "from_item_ID"))
                  {
                    if(std::find(fromIds.begin(), fromIds.end(), field.value) == fromIds.end())
                      fromIds.push_back(field.value);
                    else
                      out->error("The number of SingleItemTypeReferenceBoxes with the box type 'dimg' and from_item_ID=%u found more than once", field.value);
                  }
                }
              }
      }
    },
    {
      "Section 6.6.2.2.3\n"
      "'iovl' box: version shall be equal to 0",
      [] (Box const& root, IReport* out)
      {
        checkDerivationVersion(root, out, FOURCC("iovl"));
      }
    },
    {
      "Section 6.6.2.2.3\n"
      "'grid' box: version shall be equal to 0",
      [] (Box const& root, IReport* out)
      {
        checkDerivationVersion(root, out, FOURCC("grid"));
      }
    },
    {
      "Section 6.5.11.1\n"
      "essential shall be equal to 1 for an 'lsel' item property.",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("lsel"));
      }
    },
    {
      "Section C.2, D.2, F.2, G.2\n"
      "File extensions to identify the presence of specific image coding formats",
      [] (Box const& root, IReport* out)
      {
        if(findBoxes(root, FOURCC("avcC")).empty() && findBoxes(root, FOURCC("hvcC")).empty())
          return;

        std::string filename;

        for(auto& field : root.syms)
          if(!strcmp(field.name, "filename"))
            filename.push_back((char)field.value);

        auto extPos = filename.find_last_of('.');

        if(extPos > filename.length())
        {
          out->warning("Filename \"%s\" has no extension", filename.c_str());
          extPos = filename.length() - 1;
        }

        auto const ext = filename.substr(extPos + 1);

        auto getExtensionFromBrand = [] (Box const& root) -> std::vector<const char*> {
          for(auto& box : root.children)
            if(box.fourcc == FOURCC("ftyp"))
              for(auto& sym : box.syms)
                if(!strcmp(sym.name, "compatible_brand"))
                  switch(sym.value)
                  {
                  case FOURCC("avci"):
                    return { "avci" };
                  case FOURCC("avcs"):
                    return { "avcs" };
                  case FOURCC("heic"):
                  case FOURCC("heix"):
                  case FOURCC("heim"):
                  case FOURCC("heis"):
                    return { "heic", "hif" };
                  case FOURCC("hevc"):
                  case FOURCC("hevx"):
                  case FOURCC("hevm"):
                  case FOURCC("hevs"):
                    return { "heics", "hif" };
                  }

          auto isSequence = [] (Box const& root) -> bool {
            for(auto& box : root.children)
              if(box.fourcc == FOURCC("moov"))
                for(auto& moovChild : box.children)
                  if(moovChild.fourcc == FOURCC("trak"))
                    return true;

            return false;
          };

          if(isSequence(root))
            return { "heifs", "hif" }

          ;
          else
            return { "heif", "hif" }
          ;
        };

        bool found = false;
        auto possibleExts = getExtensionFromBrand(root);

        for(auto e : possibleExts)
          if(ext == e)
            found = true;

        std::string expected;

        for(auto e : possibleExts)
        {
          expected += e;
          expected += " ";
        }

        if(!found)
          out->warning("File extension for \"%s\" doesn't match: expecting one of '%s', got '%s'", filename.c_str(), expected.c_str(), ext.c_str());
      }
    },
    {
      "Section B.2.3.1\n"
      "essential shall be equal to 1 for an 'hvcC' item property",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("hvcC"));
      }
    },
    {
      "Section B.2.3.3\n"
      "essential shall be equal to 1 for an 'lhvC' item property",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("lhvC"));
      }
    },
    {
      "Section B.2.3.5.1\n"
      "essential shall be equal to 1 for an 'tols' item property",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("tols"));
      }
    },
    {
      "Section E.2.3\n"
      "essential shall be equal to 1 for an 'avcC' item property",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("avcC"));
      }
    },
    {
      "Section H.2.2\n"
      "essential shall be equal to 1 for an 'jpgC' item property",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("jpgC"));
      }
    },
    {
      "Section 9.3.1.1"
      "Box structure and arity for boxes defined in HEIF\n"
      "This is rather a safety check than a formal rule.",
      [] (Box const& root, IReport* out)
      {
        boxCheck(root, out, { FOURCC("ftyp") }, { FOURCC("root") }, { 1, 1 });
        boxCheck(root, out, { FOURCC("meta") }, { FOURCC("root") }, { 0, 1 });
        boxCheck(root, out, { FOURCC("hdlr") }, { FOURCC("meta") }, { 1, 1 });
        boxCheck(root, out, { FOURCC("iloc") }, { FOURCC("meta") }, { 0, INT32_MAX });
        boxCheck(root, out, { FOURCC("iinf") }, { FOURCC("meta") }, { 0, 1 });
        boxCheck(root, out, { FOURCC("infe") }, { FOURCC("iinf") }, { 0, INT32_MAX });
        boxCheck(root, out, { FOURCC("pitm") }, { FOURCC("meta") }, { 0, 1 });
      }
    },
    {
      "Section 6.4\n"
      "Box structure and arity for boxes defined in HEIF\n"
      "This is rather a safety check than a formal rule.",
      [] (Box const& root, IReport* out)
      {
        boxCheck(root, out, { FOURCC("ispe") }, { FOURCC("ipco") }, { 1, INT32_MAX }); // TODO: one per image item
        boxCheck(root, out, { FOURCC("pasp") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: at most one per image item
        boxCheck(root, out, { FOURCC("colr") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: at most one per image item
        boxCheck(root, out, { FOURCC("pixi") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: at most one per image item
        boxCheck(root, out, { FOURCC("rloc") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // mandatory, if the item has a 'tbas' item reference to another image item
        boxCheck(root, out, { FOURCC("auxC") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: one per image item // mandatory, for an image item containing an auxiliary image
        boxCheck(root, out, { FOURCC("clap") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: one per image item
        boxCheck(root, out, { FOURCC("irot") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: one per image item
        boxCheck(root, out, { FOURCC("lsel") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: one per image item
        boxCheck(root, out, { FOURCC("imir") }, { FOURCC("ipco") }, { 0, INT32_MAX }); // TODO: one per image item

        boxCheck(root, out, { FOURCC("ccst") }, visualSampleEntryFourccs, { 1, 1 });
      }
    },
    {
      "Section 10.2.1.1\n"
      "Other versions of the boxes shall not be present.",
      [] (Box const& root, IReport* out)
      {
        auto metas = findBoxes(root, FOURCC("meta"));

        for(auto meta : metas)
        {
          for(auto& field : meta->syms)
            if(!strcmp(field.name, "version"))
              if(field.value != 0)
                out->error("'meta' version shall be 0, found %lld", field.value);

          auto hdlrs = findBoxes(root, FOURCC("hdlr"));

          for(auto hdlr : hdlrs)
            for(auto& field : hdlr->syms)
              if(!strcmp(field.name, "version"))
                if(field.value != 0)
                  out->error("'hdlr' version shall be 0, found %lld", field.value);

          auto ilocs = findBoxes(root, FOURCC("iloc"));

          for(auto iloc : ilocs)
            for(auto& field : iloc->syms)
              if(!strcmp(field.name, "version"))
                if(field.value != 0 && field.value != 1 && field.value != 2)
                  out->error("'iloc' version shall be 0, 1 or 2, found %lld", field.value);

          auto iinfs = findBoxes(root, FOURCC("iinf"));

          for(auto iinf : iinfs)
            for(auto& field : iinf->syms)
              if(!strcmp(field.name, "version"))
                if(field.value != 0 && field.value != 1)
                  out->error("'iinf' version shall be 0 or 1, found %lld", field.value);

          auto infes = findBoxes(root, FOURCC("infe"));

          for(auto infe : infes)
            for(auto& field : infe->syms)
              if(!strcmp(field.name, "version"))
                if(field.value != 2 && field.value != 3)
                  out->error("'infe' version shall be 2 or 3, found %lld", field.value);

          auto pitms = findBoxes(root, FOURCC("pitm"));

          for(auto pitm : pitms)
            for(auto& field : pitm->syms)
              if(!strcmp(field.name, "version"))
                if(field.value != 0 && field.value != 1)
                  out->error("'pitm' version shall be 0 or 1, found %lld", field.value);
        }
      }
    },
    {
      "Section 10.1\n"
      "When any of the brands specified in this document is in the major_brand,\n"
      "the minor_version shall be set to zero",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "minor_version"))
                if(sym.value != 0)
                  out->error("'ftyp' minor_version shall be 0, found %lld", sym.value);
      }
    },
    {
      "Section 10.3.1.1\n"
      "Files shall contain the brand 'msf1' in the compatible brands:\n"
      "- At least one track of handler type 'pict', as defined in 7.2, is required.\n"
      "- It is required that 'iso8' is present among the compatible brands array.",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("msf1"))
                {
                  if(!checkRuleSection(specHeif, "7.2.", root))
                    out->error("'msf1' brand: this file shall conform to HEIF (section 7.2)");

                  bool iso8BrandFound = false;

                  for(auto& box : root.children)
                    if(box.fourcc == FOURCC("ftyp"))
                      for(auto& sym : box.syms)
                        if(!strcmp(sym.name, "compatible_brand"))
                          if(sym.value == FOURCC("iso8"))
                            iso8BrandFound = true;

                  if(!iso8BrandFound)
                    out->error("'msf1' brand: 'iso8' shall be present among the compatible brands array");
                }
      }
    },
    {
      "Section 10.2\n"
      "when a brand specified in 10.2 is among the compatible brands of a file,\n"
      "the requirements specified in Clause 6 shall be obeyed",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("mif1"))
                  if(!checkRuleSection(specHeif, "6", root))
                    out->error("'mif1' brand: this file shall conform to HEIF section 6, check the other errors for details");
      }
    },
    {
      "Section 10.3\n"
      "when a brand specified in 10.3 is among the compatible brands of a file,\n"
      "the requirements specified in Clause 7 shall be obeyed",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("msf1"))
                  if(!checkRuleSection(specHeif, "7", root))
                    out->error("'msf1' brand: this file shall conform to HEIF section 7, check the other errors for details");
      }
    },
  },
  isIsobmff,
};

static auto const registered = registerSpec(&specHeif);

