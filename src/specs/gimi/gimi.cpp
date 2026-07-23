#include "core/fourcc.h"
#include "core/spec.h"

#include <cstring>

bool has_compatible_brand(Box const &ftypBox, uint32_t brand);

static const SpecDesc specGimi = {
  "gimi",
  "GEOINT Imagery Media for ISR - NGA.STND.0076",
  { "isobmff" },
  {
    { "A version 1.0 NGA.STND.0076-01 file shall include the 'geo1' brand in the compatible brands list.\n ",
      "NGA.STND.0076-01_V1.0-01",
      [](Box const &root, IReport *out) {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp")) {
          out->error("'ftyp' box not found");
          return;
        }

        const Box &ftypBox = root.children[0];

        bool found = has_compatible_brand(ftypBox, FOURCC("geo1"));
        if(!found) {
          out->error("'geo1' brand not found in 'ftyp' box");
        }
      } },

    { "An NGA.STND.0076-01 conformant reader shall correctly process the 'geo1' brand's associated box content.\n",
      "NGA.STND.0076-01_V1.0-02",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        // Rule #2 only applies to readers
      } },

    { "An NGA.STND.0076-01 file shall include the 'unif' brand in the compatible brands list\n",
      "NGA.STND.0076-01_V1.0-03",
      [](Box const &root, IReport *out) {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp")) {
          out->error("'ftyp' box not found");
          return;
        }

        auto &ftypBox = root.children[0];

        bool found = has_compatible_brand(ftypBox, FOURCC("unif"));

        if(!found) {
          out->error("'unif' brand not found in 'ftyp' box");
        }
      } },

    { "An NGA.STND.0076 file shall include the 'unif' brand in the compatible brands list.\n",
      "NGA.STND.0076-01_V1.0-04",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        out->warning("TODO: Implement check for NGA.STND.0076_1.0-04");

        /* TODO: The 'unif' brand indicates the unified implementation and handling of
        IDs across file-scoped MetaBox items, tracks, track groups, and entity groups.
        */
      } },

    { "Where an NGA.STND.0076 file contains Still Imagery content, the file shall conform to the 'mif2' brand "
      "requirements.\n",
      "NGA.STND.0076-01_V1.0-05",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        out->warning("TODO: Implement check for NGA.STND.0076_1.0-05");
        /* TODO: The ‘mif2’ brand represents interoperability requirements for image and metadata items. ‘mif2’
         * represents a baseline for Still Imagery support in this standard. The HEIF standard documents the specifics
         * of the branding differences.*/
      } },

    { "Where an NGA.STND.0076 file contains Still Imagery content, the file shall include 'mif2' brand.\n",
      "NGA.STND.0076-01_V1.0-06",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        out->warning("TODO: Implement check for NGA.STND.0076_1.0-06");
        /* TODO: */
      } },

    { "Where an NGA.STND.0076 file contains image sequence content, the file shall conform to the requirements "
      "associated with the 'msf1' brand.\n",
      "NGA.STND.0076-01_V1.0-07",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        out->warning("TODO: Implement check for NGA.STND.0076_1.0-07");
        /* TODO: The 'msf1' brand indicates the presence of a HEIF defined image sequence.*/
      } },

    { "Where an NGA.STND.0076 file contains image sequence content, the file shall include the 'msf1' brand.\n",
      "NGA.STND.0076-01_V1.0-08",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        out->warning("TODO: Implement check for NGA.STND.0076_1.0-08");
        /* TODO: */
      } },

    { "Where an NGA.STND.0076-01 file contains image sequence content, the file shall include the 'msf1' brand.\n",
      "NGA.STND.0076-01_V1.0-09",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        out->warning("TODO: Implement check for NGA.STND.0076_1.0-09");
        /* TODO: The ‘isoa’ brand represents interoperability requirements for the base format as well as Motion Imagery
         * requirements for this standard. */
      } },

    { "Where an NGA.STND.0076-01 file contains the 'msf1' brand, a GIMI reader conformant with the 'msf1' brand shall "
      "correctly process the 'msf1' brand's associated box content",
      "NGA.STND.0076-01_V1.0-10",
      [](Box const &root, IReport *out) {
        (void)root;
        (void)out;
        // Applies to the Reader
      } },

  },
  isIsobmff,
};

static auto const registered = registerSpec(&specGimi);