#include "spec.h"
#include "fourcc.h"
#include "spec_utils_derivations.h"
#include <algorithm> // find
#include <cstring> // strcmp

std::vector<std::pair<uint32_t /*ItemId*/, std::string>> getAv1ItemColorspaces(Box const& root, IReport* out);

namespace
{
enum CodecDetection
{
  TRUE,
  FALSE,
  NONE,
};
CodecDetection isAvifOrMiafHevc(Box const& root)
{
  CodecDetection supported = NONE;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("ftyp"))
      for(auto& sym : box.syms)
        if(!strcmp(sym.name, "major_brand") || !strcmp(sym.name, "compatible_brand"))
          switch(sym.value)
          {
          case FOURCC("avif"): case FOURCC("avis"): case FOURCC("avio"):
            return TRUE;
          case FOURCC("heic"): case FOURCC("heix"): case FOURCC("heim"):
          case FOURCC("heis"): case FOURCC("hevc"): case FOURCC("hevx"):
          case FOURCC("hevm"): case FOURCC("hevs"):
            return NONE;
          case FOURCC("avci"): case FOURCC("avcs"):
            supported = FALSE;
            break;
          default: break;
          }

  return supported;
}

std::vector<std::pair<uint32_t /*ItemId*/, std::string>> getItemColorspaces(Box const& root, IReport* out)
{
  return getAv1ItemColorspaces(root, out);
}
}

const std::initializer_list<RuleDesc> getRulesMiafColours()
{
  static const
  std::initializer_list<RuleDesc> rulesProfiles =
  {
    {
      "Section 7.3.6.7\n"
      "Clean aperture:"
      "- when the image is 4:2:2 the horizontal cropped offset and width shall be\n"
      "  even numbers and the vertical values shall be integers\n"
      "- when the image is 4:2:0 both the horizontal and vertical cropped offsets and\n"
      "widths shall be even numbers",
      [] (Box const& root, IReport* out)
      {
        auto const supported = isAvifOrMiafHevc(root);

        if(supported == FALSE)
          out->warning("This rule is only implemented for HEVC and AV1 codecs.");

        if(supported != TRUE)
          return;

        std::map<uint32_t /*1-based*/, const Box*> clapIndices;
        std::map<uint32_t /*ItemId*/, const Box*> clapItemIds;

        // find CleanAperture boxes

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
                      if(ipcoChild.fourcc == FOURCC("clap"))
                        clapIndices[index] = &ipcoChild;

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
                        if(clapIndices.find(sym.value) != clapIndices.end())
                          clapItemIds.insert({ localItemId, clapIndices[sym.value] });
                    }
                  }

        // get colorspaces

        auto itemCsps = getItemColorspaces(root, out);

        // checks

        for(auto& item : itemCsps)
        {
          if(item.second != "YUV 4:2:2" && item.second != "YUV 4:2:0")
            continue;

          if(clapItemIds.find(item.first) == clapItemIds.end())
            continue; // no associated 'clap'

          auto clapItemId = clapItemIds[item.first];

          uint32_t cleanApertureWidthN = 0, cleanApertureWidthD = 0, cleanApertureHeightN = 0, cleanApertureHeightD = 0, horizOffN = 0, horizOffD = 0, vertOffN = 0, vertOffD = 0;

          for(auto sym : clapItemId->syms)
          {
            if(!strcmp(sym.name, "cleanApertureWidthN"))
              cleanApertureWidthN = sym.value;
            else if(!strcmp(sym.name, "cleanApertureWidthD"))
              cleanApertureWidthD = sym.value;
            else if(!strcmp(sym.name, "cleanApertureHeightN"))
              cleanApertureHeightN = sym.value;
            else if(!strcmp(sym.name, "cleanApertureHeightD"))
              cleanApertureHeightD = sym.value;
            else if(!strcmp(sym.name, "horizOffN"))
              horizOffN = sym.value;
            else if(!strcmp(sym.name, "horizOffD"))
              horizOffD = sym.value;
            else if(!strcmp(sym.name, "vertOffN"))
              vertOffN = sym.value;
            else if(!strcmp(sym.name, "vertOffD"))
              vertOffD = sym.value;
          }

          if(item.second == "YUV 4:2:2")
          {
            if(horizOffN % (2 * horizOffD))
              out->error("[ItemId=%u] YUV 4:2:2: the horizontal cropped offset shall be an even number. Found %u/%u",
                         item.first, horizOffN, horizOffD);

            if(cleanApertureWidthN % (2 * cleanApertureWidthD))
              out->error("[ItemId=%u] YUV 4:2:2: the cropped width shall be an even number. Found %u/%u",
                         item.first, cleanApertureWidthN, cleanApertureWidthD);

            if(vertOffN % vertOffD)
              out->error("[ItemId=%u] YUV 4:2:2: the vertival cropped offset shall be an integer number. Found %u/%u",
                         item.first, vertOffN, vertOffD);

            if(cleanApertureHeightN % cleanApertureHeightD)
              out->error("[ItemId=%u] YUV 4:2:2: the cropped height shall be an integer number. Found %u/%u",
                         item.first, cleanApertureHeightN, cleanApertureHeightD);
          }
          else if(item.second == "YUV 4:2:0")
          {
            if(horizOffN % (2 * horizOffD))
              out->error("[ItemId=%u] YUV 4:2:0: the horizontal cropped offset shall be an even number. Found %u/%u",
                         item.first, horizOffN, horizOffD);

            if(cleanApertureWidthN % (2 * cleanApertureWidthD))
              out->error("[ItemId=%u] YUV 4:2:0: the cropped width shall be an even number. Found %u/%u",
                         item.first, cleanApertureWidthN, cleanApertureWidthD);

            if(vertOffN % (2 * vertOffD))
              out->error("[ItemId=%u] YUV 4:2:0: the vertival cropped offset shall be an even number. Found %u/%u",
                         item.first, vertOffN, vertOffD);

            if(cleanApertureHeightN % (2 * cleanApertureHeightD))
              out->error("[ItemId=%u] YUV 4:2:0: the cropped height shall be an even number. Found %u/%u",
                         item.first, cleanApertureHeightN, cleanApertureHeightD);
          }
        }
      }
    },
    {
      "Section 7.3.11.4.1\n"
      "All input images of a grid image item shall use the same coding format,\n"
      "chroma sampling format, and the same decoder configuration",
      [] (Box const& root, IReport* out)
      {
        auto const supported = isAvifOrMiafHevc(root);

        if(supported == FALSE)
          out->warning("This rule is only implemented for HEVC and AV1 codecs.");

        if(supported != TRUE)
          return;

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

        // get colorspaces

        auto itemCsps = getItemColorspaces(root, out);

        // find coded items for each grid

        auto d = getDerivationsInfo(root, FOURCC("dimg"));

        for(auto gridItemId : gridItemIds)
        {
          std::string colorspace;

          for(auto& iref : d.itemRefs)
            if(iref.first == gridItemId)
            {
              for(auto toItemId : iref.second)
              {
                if(std::find(iref.second.begin(), iref.second.end(), toItemId) == iref.second.end())
                {
                  out->error("[ItemId=%u, gridItemId=%u] no colorspace information attached", toItemId, gridItemId);
                  continue;
                }

                for(auto& item : itemCsps)
                {
                  if(item.first != toItemId)
                    continue;

                  if(colorspace.empty())
                    colorspace = item.second;
                  else if(colorspace != item.second)
                    out->error("[ItemId=%u, gridItemId=%u] found colorspace \"%s\" instead of previous found \"%s\"",
                               item.first, gridItemId, item.second.c_str(), colorspace.c_str());
                }
              }
            }
        }
      }
    },
    {
      "Section 7.3.11.4.1\n"
      "The tile size is restricted according to the chroma sampling format of the\n"
      "input images; the cropping shall select an integer number of samples for\n"
      "all planes, and result in an output image that also includes an integer number\n"
      "of samples for all planes",
      [] (Box const& root, IReport* out)
      {
        auto const supported = isAvifOrMiafHevc(root);

        if(supported == FALSE)
          out->warning("This rule is only implemented for HEVC and AV1 codecs.");

        if(supported != TRUE)
          return;

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

        // get colorspaces

        auto itemCsps = getItemColorspaces(root, out);

        // find coded items for each grid

        auto d = getDerivationsInfo(root, FOURCC("dimg"));

        for(auto gridItemId : gridItemIds)
        {
          std::string colorspace;

          for(auto& iref : d.itemRefs)
          {
            if(iref.first == gridItemId)
            {
              for(auto toItemId : iref.second)
              {
                if(std::find(iref.second.begin(), iref.second.end(), toItemId) == iref.second.end())
                {
                  out->error("[ItemId=%u, gridItemId=%u] no colorspace information attached", toItemId, gridItemId);
                  continue;
                }

                for(auto& item : itemCsps)
                {
                  if(item.first != toItemId)
                    continue;

                  if(colorspace.empty())
                  {
                    colorspace = item.second;
                    break; // stop at first tile: consistency is checked elsewhere
                  }
                }
              }
            }

            if(!colorspace.empty())
              break;
          }

          Resolution resIspe;

          for(auto& iref : d.itemRefs)
          {
            if(iref.first == gridItemId)
              for(auto itemId : iref.second)
              {
                if(resIspe.width == Resolution().width)
                {
                  resIspe = d.itemRes[itemId];
                  break; // stop at first tile: consistency is checked elsewhere
                }
              }

            if(resIspe.width != -1)
              break;
          }

          if(colorspace == "YUV 4:2:0")
            if((resIspe.width % 2) || (resIspe.height % 2))
              out->error("[gridItemId=%u] for YUV 4:2:0 width(%d) and height(%d) should be even", gridItemId, resIspe.width, resIspe.height);

          if(colorspace == "YUV 4:2:2")
            if(resIspe.width % 2)
              out->error("[gridItemId=%u] for YUV 4:2:0 width(%d) should be even", gridItemId, resIspe.width);
        }
      }
    },
    {
      "Section 7.3.5.1\n"
      "Depth maps and alpha planes [...] if [...] encoded in colour [...] shall be\n"
      "encoded in a colour format with a luma plane and chroma planes",
      [] (Box const& root, IReport* out)
      {
        auto const supported = isAvifOrMiafHevc(root);

        if(supported == FALSE)
          out->warning("This rule is only implemented for HEVC and AV1 codecs.");

        if(supported != TRUE)
          return;

        // nothing to do for AV1, AVC and HEVC which are YCbCr-based
      }
    }
  };

  return rulesProfiles;
}

