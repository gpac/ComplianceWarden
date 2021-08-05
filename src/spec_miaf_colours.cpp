#include "spec.h"
#include "fourcc.h"
#include <cstring> // strcmp

namespace
{
bool isCodecParsable(Box const& root)
{
  for(auto& box : root.children)
    if(box.fourcc == FOURCC("ftyp"))
      for(auto& sym : box.syms)
        if(!strcmp(sym.name, "major_brand") || !strcmp(sym.name, "compatible_brand"))
          switch(sym.value)
          {
          case FOURCC("avif"): case FOURCC("avis"): case FOURCC("avio"):
            return true;
          default: break;
          }

  return false;
}
}

const std::initializer_list<RuleDesc> getRulesMiafColours()
{
  static const
  std::initializer_list<RuleDesc> rulesProfiles =
  {
    {
      "Section 7.3.6.7\n"
      "when the image is 4:0:0 (monochrome) or 4:4:4, the horizontal and\n"
      "vertical cropped offsets and widths shall be integers",
      [] (Box const& root, IReport*)
      {
        if(!isCodecParsable(root))
          return;
      }
    },
    {
      "Section 7.6.6.7\n"
      "when the image is 4:2:2 the horizontal cropped offset and width shall be\n"
      "even numbers and the vertical values shall be integers",
      [] (Box const& root, IReport*)
      {
        if(!isCodecParsable(root))
          return;
      }
    },
    {
      "Section 7.3.6.7\n"
      "when the image is 4:2:0 both the horizontal and vertical cropped offsets and\n"
      "widths shall be even numbers",
      [] (Box const& root, IReport*)
      {
        if(!isCodecParsable(root))
          return;
      }
    },
    {
      "Section 7.3.11.4.1\n"
      "All input images of a grid image item shall use the same coding format,\n"
      "chroma sampling format, and the same decoder configuration",
      [] (Box const& root, IReport*)
      {
        if(!isCodecParsable(root))
          return;
      }
    },
    {
      "Section 7.3.11.4.1\n"
      "The tile size is restricted according to the chroma sampling format of the\n"
      "input images; the cropping shall select an integer number of samples for\n"
      "all planes, and result in an output image that also includes an integer number\n"
      "of samples for all planes",
      [] (Box const& root, IReport*)
      {
        if(!isCodecParsable(root))
          return;
      }
    },
    {
      "Section 7.3.5.1\n"
      "Depth maps and alpha planes [...] if [...] encoded in colour [...] shall be\n"
      "encoded in a colour format with a luma plane and chroma planes",
      [] (Box const& root, IReport*)
      {
        if(!isCodecParsable(root))
          return;
      }
    }
  };

  return rulesProfiles;
}

