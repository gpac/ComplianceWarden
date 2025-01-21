#include "core/fourcc.h"
#include "core/spec.h"

#include <cstring>

static const SpecDesc specGimi = {
  "gimi",
  "GEOINT Imagery Media for ISR - NGA.STND.0076",
  { "isobmff" },
  {
    { "Requirement NGA.STND.0076_1.0-02\n"
      "An NGA.STND.0076_1.0 file shall include the 'geo1' brand in the\n"
      "compatible brands list.\n",
      [](Box const &root, IReport *out) {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp")) {
          out->error("'ftyp' box not found");
          return;
        }

        auto &ftypBox = root.children[0];

        bool found = false;

        for(auto &brand : ftypBox.syms)
          if(!strcmp(brand.name, "compatible_brand") && brand.value == FOURCC("geo1"))
            found = true;

        if(!found)
          out->error("'geo1' brand not found in 'ftyp' box");
      } },
    { "Requirement NGA.STND.0076_1.0-03\n"
      "An NGA.STND.0076_1.0 file shall include the 'unif' brand in the\n"
      "compatible brands list.\n",
      [](Box const &root, IReport *out) {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp")) {
          out->error("'ftyp' box not found");
          return;
        }

        auto &ftypBox = root.children[0];

        bool found = false;

        for(auto &brand : ftypBox.syms)
          if(!strcmp(brand.name, "compatible_brand") && brand.value == FOURCC("unif"))
            found = true;

        if(!found)
          out->error("'unif' brand not found in 'ftyp' box");
      } },
  },
  isIsobmff,
};

static auto const registered = registerSpec(&specGimi);