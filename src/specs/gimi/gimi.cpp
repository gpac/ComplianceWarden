

#include "gimi_utils.h"

bool has_compatible_brand(const Box &ftypBox, uint32_t brand);

static const SpecDesc specGimi = {
  "gimi",
  "GEOINT Imagery Media for ISR - NGA.STND.0076",
  { "isobmff" },
  {
    { "A version 1.0 NGA.STND.0076-01 file shall include the 'geo1' brand in the compatible brands list.\n ",
      "NGA.STND.0076-01_V1.0-01",
      [](const Box &root, IReport *out) {
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
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        // Rule #2 only applies to readers
      } },

    { "An NGA.STND.0076-01 file shall include the 'unif' brand in the compatible brands list\n",
      "NGA.STND.0076-01_V1.0-03",
      [](const Box &root, IReport *out) {
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

    { "An NGA.STND.0076-01 file shall conform to the requirements of the ISOBMFF 'unif' brand.\n",
      "NGA.STND.0076-01_V1.0-04",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        printf("TODO: Implement check for NGA.STND.0076_1.0-04\n");

        // See ISO/IEC 14496-12 Annex E.18
        // Each identifier identifies at most one thing. Example, the same identifier is not used for two different
        // things.
        /* TODO: The 'unif' brand indicates the unified implementation and handling of
        IDs across file-scoped MetaBox items, tracks, track groups, and entity groups.
        */
      } },

    { "Where an NGA.STND.0076 file contains Still Imagery content, the file shall conform to the 'mif2' brand "
      "requirements.\n",
      "NGA.STND.0076-01_V1.0-05",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        printf("TODO: Implement check for NGA.STND.0076_1.0-05\n");
        /* TODO: The ‘mif2’ brand represents interoperability requirements for image and metadata items. ‘mif2’
         * represents a baseline for Still Imagery support in this standard. The HEIF standard documents the specifics
         * of the branding differences.*/
      } },

    { "Where an NGA.STND.0076 file contains Still Imagery content, the file shall include 'mif2' brand.\n",
      "NGA.STND.0076-01_V1.0-06",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        printf("TODO: Implement check for NGA.STND.0076_1.0-06\n");
        /* TODO: */
      } },

    { "Where an NGA.STND.0076 file contains image sequence content, the file shall conform to the requirements "
      "associated with the 'msf1' brand.\n",
      "NGA.STND.0076-01_V1.0-07",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        printf("TODO: Implement check for NGA.STND.0076_1.0-07\n");
        /* TODO: The 'msf1' brand indicates the presence of a HEIF defined image sequence.*/
      } },

    { "Where an NGA.STND.0076 file contains image sequence content, the file shall include the 'msf1' brand.\n",
      "NGA.STND.0076-01_V1.0-08",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        printf("TODO: Implement check for NGA.STND.0076_1.0-08\n");
        /* TODO: */
      } },

    { "Where an NGA.STND.0076-01 file contains image sequence content, the file shall include the 'msf1' brand.\n",
      "NGA.STND.0076-01_V1.0-09",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        printf("TODO: Implement check for NGA.STND.0076_1.0-09\n");
        /* TODO: The ‘isoa’ brand represents interoperability requirements for the base format as well as Motion
         * Imagery requirements for this standard. */
      } },

    { "Where an NGA.STND.0076-01 file contains the 'msf1' brand, a GIMI reader conformant with the 'msf1' brand "
      "shall "
      "correctly process the 'msf1' brand's associated box content",
      "NGA.STND.0076-01_V1.0-10",
      [](const Box &root, IReport *out) {
        (void)root; // Resolves unused parameter warning
        (void)out; // Resolves unused parameter warning
        // Applies to the Reader
      } },

    { "Image items shall associate with an ItemContentID property.", //
      "NGA.STND.0076-01_V1.0-63",
      [](const Box &root, IReport *out) {
        (void)out; // Resolves unused parameter warning

        std::vector<const Box *> metaboxes = get_all_metaboxes(root);

        for(const Box *metabox : metaboxes) {
          std::vector<const Box *> items = get_image_items(*metabox);
          std::vector<const Box *> itemProperties = get_item_properties(*metabox);
          std::vector<const Box *> itemPropertyAssociations = get_item_property_associations(*metabox);

          // Get All Item ContentID Properties
          std::vector<const Box *> itemContentIDProperties;
          for(const Box *property : itemProperties) {
            if(property->fourcc == FOURCC("uuid")) {
              // TODO: parse uuid box!!!
              // itemContentIDProperties.push_back(property);
            }
          }

          // WIP
          for(const Box *item : items) {
            const Symbol &item_ID = get_symbol_by_name(*item, "item_ID");
            printf("item_ID: %ld\n", item_ID.value);
          }

          for(const Box *association : itemPropertyAssociations) {
            const Symbol &association_count = get_symbol_by_name(*association, "association_count");
            const Symbol &item_ID = get_symbol_by_name(*association, "item_ID");

            printf("association_count: %ld, item_ID: %ld\n", association_count.value, item_ID.value);
          }
        }
      } },

  },
  isIsobmff,
};

static auto const registered = registerSpec(&specGimi);