#include "spec.h"
#include "fourcc.h"
#include <cstring> // strcmp

std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);

const std::initializer_list<RuleDesc> getRulesAvifProfiles(const SpecDesc & /*spec*/)
{
  static const std::initializer_list<RuleDesc> rulesProfiles =
  {
    {
      "Section 7.2\n"
      "AVIF Baseline Profile",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MA1B"))
                  found = true;

        if(!found)
          return;

        // applies to all AV1 Image Items and all AV1 Image Sequences

        auto av1Cs = findBoxes(root, FOURCC("av1C"));

        for(auto& av1C : av1Cs)
        {
          for(auto& sym : av1C->syms)
          {
            if(!strcmp(sym.name, "seq_profile"))
              if(sym.value > 0 /*Main*/)
                out->error("Baseline profile requires AV1 Main Profile");

            if(!strcmp(sym.name, "seq_level_idx_0"))
              if(sym.value > 13 /*5.1*/)
                out->error("Baseline profile requires AV1 level 5.1 or lower");
          }
        }
      }
    },
    {
      "Section 7.3\n"
      "AVIF Advanced Profile",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MA1A"))
                  found = true;

        if(!found)
          return;

        // image items
        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
          {
            auto av1Cs = findBoxes(root, FOURCC("av1C"));

            for(auto& av1C : av1Cs)
            {
              for(auto& sym : av1C->syms)
              {
                if(!strcmp(sym.name, "seq_profile"))
                  if(sym.value > 1 /*High*/)
                    out->error("Advanced profile requires AV1 High Profile for image items");

                if(!strcmp(sym.name, "seq_level_idx_0"))
                  if(sym.value > 16 /*6*/)
                    out->error("Advanced profile requires AV1 level 6 or lower for image items");
              }
            }
          }
        }

        // image sequences

        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("moov"))
          {
            auto av1Cs = findBoxes(root, FOURCC("av1C"));

            for(auto& av1C : av1Cs)
            {
              for(auto& sym : av1C->syms)
              {
                if(!strcmp(sym.name, "seq_profile"))
                  if(sym.value > 1 /*Main or High*/)
                    out->error("Advanced profile requires AV1 Main or High Profiles for image sequences");

                if(!strcmp(sym.name, "seq_level_idx_0"))
                  if(sym.value > 13 /*5.1*/)
                    out->error("Advanced profile requires AV1 level 5.1 or lower for image sequences");
              }
            }
          }
        }
      }
    }
  };
  return rulesProfiles;
}

