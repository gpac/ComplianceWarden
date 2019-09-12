#include "spec.h"
#include "fourcc.h"
#include <cstring>
#include <map>
#include <vector>

struct Resolution
{
  int width, height;
};

struct Derivations
{
  std::map<int /*propertyIndex*/, Resolution> resolutions;
  std::map<uint32_t /*item_ID*/, Resolution> itemRes;
  std::map<uint32_t /*item_ID*/, std::vector<uint32_t> /*thmb/dimg item_ID*/> itemRefs;
};

static Derivations getDerivationsInfo(Box const& root, uint32_t irefTypeFourcc)
{
  Derivations d;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco"))
            {
              int propertyIndex = 1;

              for(auto& ipcoChild : iprpChild.children)
              {
                if(ipcoChild.fourcc == FOURCC("ispe"))
                {
                  int width = 0, height = 0;

                  for(auto& sym : ipcoChild.syms)
                  {
                    if(!strcmp(sym.name, "image_width"))
                      width = (int)sym.value;
                    else if(!strcmp(sym.name, "image_height"))
                      height = (int)sym.value;
                  }

                  d.resolutions.insert({ propertyIndex, { width, height }
                                       });
                }

                propertyIndex++;
              }
            }

  // find 'ipma' to associate item_ID with resolution

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipma"))
            {
              int item_ID = 0;

              for(auto& sym : iprpChild.syms)
              {
                if(!strcmp(sym.name, "item_ID"))
                  item_ID = (int)sym.value;
                else if(!strcmp(sym.name, "property_index"))
                  if(d.resolutions.find(sym.value) != d.resolutions.end())
                    d.itemRes.insert({ item_ID, d.resolutions[sym.value] });
              }
            }

  // find 'iref' to find thumbnails

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iref"))
        {
          bool parsing = false;
          uint32_t fromItemId = 0;

          for(auto& sym : metaChild.syms)
          {
            if(!strcmp(sym.name, "box_type"))
            {
              parsing = false;

              if(sym.value == irefTypeFourcc)
                parsing = true;
            }

            if(parsing)
            {
              if(!strcmp(sym.name, "from_item_ID"))
              {
                auto it = d.itemRefs.find(sym.value);

                if(it == d.itemRefs.end())
                {
                  d.itemRefs.insert({ sym.value, {}
                                    });
                  it = d.itemRefs.find(sym.value);
                }

                fromItemId = sym.value;
              }
              else if(!strcmp(sym.name, "to_item_ID"))
              {
                auto it = d.itemRefs.find(fromItemId);
                it->second.push_back(sym.value);
              }
            }
          }
        }

  return d;
}

const std::initializer_list<RuleDesc> getRulesNumPixels()
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
        auto d = getDerivationsInfo(root, FOURCC("thmb"));

        // check
        uint64_t lastNumPixels = 0;

        for(auto& item : d.itemRefs)
        {
          bool foundLargerThmb = false;
          lastNumPixels = 0;

          for(auto& thmb : item.second)
          {
            uint64_t numPixels = d.itemRes[thmb].width * d.itemRes[thmb].height;

            if(numPixels > lastNumPixels)   // "next larger"
            {
              if(lastNumPixels != 0)
              {
                foundLargerThmb = true;

                if(numPixels > 200 * lastNumPixels)
                  out->error("Next larger thumbnail has %d times more pixels (resolutions: %llu vs %dx%d). Limit is 200.",
                             numPixels / lastNumPixels, lastNumPixels, d.itemRes[thmb].width, d.itemRes[thmb].height);
              }

              lastNumPixels = numPixels;
            }
          }

          if(!foundLargerThmb)
          {
            uint64_t numPixels = d.itemRes[item.first].width * d.itemRes[item.first].height;

            if((lastNumPixels != 0) && (numPixels > 200 * lastNumPixels))
              out->error("Next larger is a master image that has %d times more pixels (resolutions: %llu vs %dx%d). Limit is 200.",
                         numPixels / lastNumPixels, lastNumPixels, d.itemRes[item.first].width, d.itemRes[item.first].height);
          }
        }
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
                  out->error("Tile widths shall be greater than or equal to 64. Found %d.", d.itemRes[dimg].width);

                if(d.itemRes[dimg].height < 64)
                  out->error("Tile heights shall be greater than or equal to 64. Found %d.", d.itemRes[dimg].height);

                if(d.itemRes[dimg].width % 64)
                  out->error("Tile widths should be a multiple of 64. Found %d.", d.itemRes[dimg].width);

                if(d.itemRes[dimg].height % 64)
                  out->error("Tile heights should be a multiple of 64. Found %d.", d.itemRes[dimg].height);
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
      "or thumbnail, whose sum does not exceed 128,000,000 pixels",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 8.4\n"
      "There shall be no greater than a factor of 200 between the total number of\n"
      "pixels in the overlay, grid image or thumbnail that is less than 128,000,000\n"
      "pixels, and the next larger alternative",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
  };

  return rulesNumPixels;
}

