#include "spec.h"
#include "fourcc.h"
#include "spec_utils_derivations.h"
#include <cstring>
#include <vector>

namespace
{
void validateNextLarger(Box const& root, IReport* out, uint32_t fourcc)
{
  auto d = getDerivationsInfo(root, fourcc);

  // check
  uint64_t lastNumPixels = 0;

  for(auto& fromItem : d.itemRefs)
  {
    bool foundLargerItem = false;
    lastNumPixels = 0;

    for(auto& toItem : fromItem.second)
    {
      uint64_t numPixels = d.itemRes[toItem].width * d.itemRes[toItem].height;

      if(numPixels > lastNumPixels)   // "next larger"
      {
        if(lastNumPixels != 0)
        {
          foundLargerItem = true;

          if(numPixels > 200 * lastNumPixels)
            out->error("Next larger \"%s\" has %d times more pixels (resolutions: %llu vs %dx%d). Limit is 200",
                       toString(fourcc).c_str(), numPixels / lastNumPixels, lastNumPixels, d.itemRes[toItem].width, d.itemRes[toItem].height);
        }

        lastNumPixels = numPixels;
      }
    }

    if(!foundLargerItem)
    {
      uint64_t numPixels = d.itemRes[fromItem.first].width * d.itemRes[fromItem.first].height;

      if((lastNumPixels != 0) && (numPixels > 200 * lastNumPixels))
        out->error("Next larger is a master image that has %d times more pixels (resolutions: %llu vs %dx%d). Limit is 200",
                   numPixels / lastNumPixels, lastNumPixels, d.itemRes[fromItem.first].width, d.itemRes[fromItem.first].height);
    }
  }
}
}

const std::initializer_list<RuleDesc> getRulesMiafNumPixels()
{
  static const
  std::initializer_list<RuleDesc> rulesNumPixels =
  {
    {
      "Section 7.3.3\n"
      "There shall be no greater than a factor of 200 between the total number of pixels\n"
      "in a MIAF thumbnail image item and the next larger MIAF thumbnail image item, or\n"
      "the associated MIAF master image item if there is no larger MIAF thumbnail image\n"
      "item",
      [] (Box const& root, IReport* out)
      {
        validateNextLarger(root, out, FOURCC("thmb"));
      }
    },
    {
      "Section 7.3.11.4.2\n"
      "Any grid image in a MIAF file shall also conform to the following constraints:\n"
      "- The tile_width shall be greater than or equal to 64, and should be a multiple\n"
      "  of 64.\n"
      "- The tile_height shall be greater than or equal to 64, and should be a multiple\n"
      "  of 64",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> gridItemIds;

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

        auto d = getDerivationsInfo(root, FOURCC("dimg"));

        // check
        for(auto grid : gridItemIds)
        {
          for(auto& item : d.itemRefs)
          {
            if(item.first == grid)
            {
              for(auto& dimg : item.second)
              {
                if(d.itemRes[dimg].width < 64)
                  out->error("Tile widths shall be greater than or equal to 64. Found %d", d.itemRes[dimg].width);

                if(d.itemRes[dimg].height < 64)
                  out->error("Tile heights shall be greater than or equal to 64. Found %d", d.itemRes[dimg].height);

                if(d.itemRes[dimg].width % 64)
                  out->warning("Tile widths should be a multiple of 64. Found %d", d.itemRes[dimg].width);

                if(d.itemRes[dimg].height % 64)
                  out->warning("Tile heights should be a multiple of 64. Found %d", d.itemRes[dimg].height);
              }
            }
          }
        }
      }
    },
    {
      "Section 8.4\n"
      "When an overlay or grid derived image item is the primary item or in an alternate\n"
      "group that also contains the primary item, if the sum of the pixel counts of the\n"
      "input images of the derived image item exceeds 128,000,000 pixels there shall be\n"
      "an alternate such image of the same type (i.e. overlay or grid) in the same group\n"
      "or thumbnail, whose sum does not exceed 128,000,000 pixels\n\n"
      "There shall be no greater than a factor of 200 between the total number of\n"
      "pixels in the overlay, grid image or thumbnail that is less than 128,000,000\n"
      "pixels, and the next larger alternative",
      [] (Box const& root, IReport* out)
      {
        validateNextLarger(root, out, FOURCC("grid"));
        validateNextLarger(root, out, FOURCC("iovl"));
      }
    },
  };

  return rulesNumPixels;
}

