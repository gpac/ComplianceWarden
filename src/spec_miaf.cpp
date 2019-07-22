#include <cstring>
#include <vector>
#include "spec.h"

static const SpecDesc spec =
{
  "miaf",
  "MIAF - ISO/IEC 23000-22 - w18260 FDIS - Jan 2019",
  {
    {
      "The file-level MetaBox shall always be present (see 7.2.1.4).\n"
      "The MetaBox shall be present at the file-level",
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
      "The HandlerBox shall be the first contained box within the MetaBox.",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
          {
            if(box.children.empty() || box.children[0].fourcc != FOURCC("hdlr"))
              out->error("The HandlerBox shall be the first contained box within the MetaBox.");
          }
      }
    },
    {
      "The FileTypeBox shall contain, in the compatible_brands list, "
      "the following (in any order): 'mif1' (specified in ISO/IEC 23008-12) "
      "[and] brand(s) identifying conformance to this document (specified in 10)."
      "[...]"
      "Files conforming to the general restrictions in clause 7 shall include "
      "the brand 'miaf' in the compatible_brands in the FileTypeBox.",
      [] (Box const& root, IReport* out)
      {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
        {
          out->error("ftyp box not found");
          return;
        }

        auto& ftypBox = root.children[0];

        bool found = false;

        struct Brand { char brand[5];
        } mif1 { "mif1" }, miaf { "miaf" };
        std::vector<Brand> expected = { miaf, mif1 };

        for(auto& brand : ftypBox.syms)
        {
          if(strcmp(brand.name, "compatible_brand"))
            continue;

          if(brand.value != FOURCC(expected.back().brand))
          {
            auto* v = 3 + (char*)&brand.value;
            char got[5] = { *(v--), *(v--), *(v--), *v, 0 };
            out->error("Expected compatible brand '%s', got '%s'", expected.back().brand, got);
            break;
          }

          expected.pop_back();

          if(expected.empty())
          {
            found = true;
            break;
          }
        }

        if(!found)
          out->error("compatible_brands list order is not conformant (%d not found).", (int)expected.size());
      },
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

