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
      "An NGA.STND.0076_1.0 file shall include the 'unif' brand in the compatible brands list.\n",
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

    { "Requirement NGA.STND.0076_1.0-04\n"
      "An NGA.STND.0076 file shall include the 'unif' brand in the compatible brands list.",
      [](Box const &root, IReport *out) {
        /* TODO: The 'unif' brand indicates the unified implementation and handling of
        IDs across file-scoped MetaBox items, tracks, track groups, and entity groups.
        */
      } },

    { "Requirement NGA.STND.0076_1.0-05\n"
      "Where an NGA.STND.0076 file contains Still Imagery content, the file shall conform to the 'mif2' brand "
      "requirements.",
      [](Box const &root, IReport *out) {
        /* TODO: The ‘mif2’ brand represents interoperability requirements for image and metadata items. ‘mif2’
         * represents a baseline for Still Imagery support in this standard. The HEIF standard documents the specifics
         * of the branding differences.*/
      } },

    { "Requirement NGA.STND.0076_1.0-06\n"
      "Where an NGA.STND.0076 file contains Still Imagery content, the file shall include 'mif2' brand.",
      [](Box const &root, IReport *out) {
        /* TODO: */
      } },

    { "Requirement NGA.STND.0076_1.0-07\n"
      "Where an NGA.STND.0076 file contains image sequence content, the file shall conform to the requirements "
      "associated with the 'msf1' brand.",
      [](Box const &root, IReport *out) {
        /* TODO: The 'msf1' brand indicates the presence of a HEIF defined image sequence.*/
      } },

    { "Requirement NGA.STND.0076_1.0-08\n"
      "Where an NGA.STND.0076 file contains image sequence content, the file shall include the 'msf1' brand.",
      [](Box const &root, IReport *out) {
        /* TODO: */
      } },

    { "Requirement NGA.STND.0076_1.0-09\n"
      "Where an NGA.STND.0076 file contains Motion Imagery content, the file shall conform to the requirements "
      "associated with the 'isoa' brand.",
      [](Box const &root, IReport *out) {
        /* TODO: The ‘isoa’ brand represents interoperability requirements for the base format as well as Motion Imagery
         * requirements for this standard. */
      } },

    { "Requirement NGA.STND.0076_1.0-10\n"
      "NGA.STND.0076_1.0-10	Where an NGA.STND.0076 file contains Motion Imagery content, the file shall include "
      "the 'isoa' brand.",
      [](Box const &root, IReport *out) {
        /* TODO: */
      } },
  },
  isIsobmff,
};

static auto const registered = registerSpec(&specGimi);