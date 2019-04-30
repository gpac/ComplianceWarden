#include <cstring>
#include "spec.h"

static const SpecDesc spec =
{
  "heif",
  "HEIF - ISO/IEC 23008-12 - First edition 2017-12",
  {
    {
      "This is a placeholder rule",
      [] (Box const &, IReport*)
      {
      }
    },
    {
      "The FileTypeBox shall contain, in the compatible_brands list, "
      "the following (in any order): 'mif1' (specified in ISO/IEC 23008-12) "
      "[and] brand(s) identifying conformance to this document (specified in 10).",
      [] (Box const& root, IReport* out)
      {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
        {
          out->error("ftyp box not found");
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
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

