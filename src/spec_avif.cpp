#include "spec.h"
#include "fourcc.h"
#include <algorithm> // std::find
#include <cstring> // strcmp
#include <map>

#if 0
namespace
{
void parse_seq_header()
{
  // TODO
}
}
#endif

extern void checkEssential(Box const& root, IReport* out, uint32_t fourcc);

static const SpecDesc spec =
{
  "avif",
  "AVIF v1.0.0, 19 February 2019\n"
  "https://aomediacodec.github.io/av1-avif/",
  { "heif" },
  {
    {
      "Section 2.1 AV1 Image Item\n"
      "The AV1 Image Item shall be associated with an AV1 Item Configuration Property.",
      [] (Box const& root, IReport* out)
      {
        std::map<uint32_t /*itemId*/, bool> av1ImageItemIDs;

        // Find AV1 Image Items
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
                        if(sym.value == FOURCC("av01"))
                          av1ImageItemIDs.insert({ itemId, false });
                    }
                  }

        // AV1 Image Item shall be associated with an AV1 Item Configuration Property
        std::vector<uint32_t> av1cPropertyIndex; /* 1-based */

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipco"))
                    for(uint32_t i = 1; i <= iprpChild.children.size(); ++i)
                      if(iprpChild.children[i - 1].fourcc == FOURCC("av1C"))
                        av1cPropertyIndex.push_back(i);

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    uint32_t itemId = 0;

                    for(auto& sym : iprpChild.syms)
                    {
                      if(!strcmp(sym.name, "item_ID"))
                        itemId = sym.value;
                      else if(!strcmp(sym.name, "property_index"))
                        if(std::find(av1cPropertyIndex.begin(), av1cPropertyIndex.end(), sym.value) != av1cPropertyIndex.end())
                          av1ImageItemIDs[itemId] = true;
                    }
                  }

        for(auto& item : av1ImageItemIDs)
          if(item.second == false)
            out->error("AV1 Image Item (ID=%u) shall be associated with an AV1 Item Configuration Property", item.first);
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data shall be identical to the content of an AV1 Sample marked as sync",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data shall have exactly one Sequence Header OBU.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data should have its still_picture flag set to 1.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data should have its reduced_still_picture_header flag set to 1.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "If a Sequence Header OBU is present in the AV1CodecConfigurationBox, it shall match the\n"
      "Sequence Header OBU in the AV1 Image Item Data.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "The values of the fields in the AV1CodecConfigurationBox shall match those of the\n"
      "Sequence Header OBU in the AV1 Image Item Data.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "Metadata OBUs, if present, shall match the values given in other item properties, such as\n"
      "the PixelInformationProperty or ColourInformationBox.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "AV1 Item Configuration Property [...] shall be marked as essential.",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("av1C"));
      }
    },
#if 0
    {
      "Section 4. Auxiliary Image Items\n"
      "The mono_chrome field in the Sequence Header OBU shall be set to 1.",
      [] (Box const& root, IReport* out)
      {
        // TODO: is an AV1 Image Item OR  AV1 Image Sequence => Check against 2.1
        // TODO: check Auxiliary

        // TODO: parse seq hdr until color config
      }
    },
#endif
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

