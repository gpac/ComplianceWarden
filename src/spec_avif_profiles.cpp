#include "spec.h"
#include "fourcc.h"
#include <cstring> // strcmp
#include <map>

std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);

namespace
{
std::map<uint32_t /*ItemId*/, const Box* /*av1C*/> getAv1CPerItemId(Box const& root)
{
  std::map<uint32_t, const Box*> av1cPropertyIndex, av1cPerItemId;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco"))
              for(uint32_t i = 1; i <= iprpChild.children.size(); ++i)
                if(iprpChild.children[i - 1].fourcc == FOURCC("av1C"))
                  av1cPropertyIndex.insert({ i, &iprpChild.children[i - 1] });

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
                  for(auto& a : av1cPropertyIndex)
                    av1cPerItemId.insert({ localItemId, a.second });
              }
            }

  return av1cPerItemId;
}

std::map<uint32_t /*TrackId*/, const Box* /*av1C*/> getAv1CPerTrackId(Box const& root)
{
  std::map<uint32_t, const Box*> av1cPerTrackId;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("moov"))
      for(auto& moovChild : box.children)
        if(moovChild.fourcc == FOURCC("trak"))
        {
          uint32_t trackId = 0;

          for(auto& trakChild : moovChild.children)
            if(trakChild.fourcc == FOURCC("tkhd"))
            {
              for(auto& sym : trakChild.syms)
                if(!strcmp(sym.name, "track_ID"))
                  trackId = sym.value;
            }
            else if(trakChild.fourcc == FOURCC("mdia"))
              for(auto& mdiaChild : trakChild.children)
                if(mdiaChild.fourcc == FOURCC("minf"))
                  for(auto& minfChild : mdiaChild.children)
                    if(minfChild.fourcc == FOURCC("stbl"))
                      for(auto& stblChild : minfChild.children)
                        if(stblChild.fourcc == FOURCC("stsd"))
                          for(auto& stsdChild : stblChild.children)
                            if(stsdChild.fourcc == FOURCC("av01"))
                              for(auto& sampleEntryChild : stsdChild.children)
                                if(sampleEntryChild.fourcc == FOURCC("av1C"))
                                  av1cPerTrackId.insert({ trackId, &sampleEntryChild });
        }

  return av1cPerTrackId;
}
}

const std::initializer_list<RuleDesc> getRulesAvifProfiles(const SpecDesc & /*spec*/)
{
  static const std::initializer_list<RuleDesc> rulesProfiles =
  {
    {
      "Section 7.2\n"
      "AVIF Baseline Profile (MA1B):\n"
      "- Image items and image sequences: AV1 profile shall be Main Profile and\n"
      "  level shall be 5.1 or lower",
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

        // image items

        auto imageItemAv1Cs = getAv1CPerItemId(root);

        for(auto& av1C : imageItemAv1Cs)
        {
          for(auto& sym : av1C.second->syms)
          {
            if(!strcmp(sym.name, "seq_profile"))
              if(sym.value > 0 /*Main*/)
                out->error("Item ID=%u (image item): Baseline Profile requires AV1 Main Profile", av1C.first);

            if(!strcmp(sym.name, "seq_level_idx_0"))
              if(sym.value > 13 /*5.1*/)
                out->error("Item ID=%u (image item): Baseline Profile requires AV1 level 5.1 or lower", av1C.first);
          }
        }

        // image sequences

        auto imageSequenceAv1Cs = getAv1CPerTrackId(root);

        for(auto& av1C : imageSequenceAv1Cs)
        {
          for(auto& sym : av1C.second->syms)
          {
            if(!strcmp(sym.name, "seq_profile"))
              if(sym.value > 0 /*Main*/)
                out->error("Item ID=%u (image sequence): Baseline Profile requires AV1 Main Profile", av1C.first);

            if(!strcmp(sym.name, "seq_level_idx_0"))
              if(sym.value > 13 /*5.1*/)
                out->error("Item ID=%u (image sequence): Baseline Profile requires AV1 level 5.1 or lower", av1C.first);
          }
        }
      }
    },
    {
      "Section 7.3\n"
      "AVIF Advanced Profile (MA1A):\n"
      "- Image items: AV1 profile shall be High Profile and level shall be 6 or lower\n"
      "- Image sequences: AV1 profile shall be Main Profile and level shall be 5.1 or\n"
      "  lower",
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
        auto imageItemAv1Cs = getAv1CPerItemId(root);

        for(auto& av1C : imageItemAv1Cs)
        {
          for(auto& sym : av1C.second->syms)
          {
            if(!strcmp(sym.name, "seq_profile"))
              if(sym.value > 1 /*High*/)
                out->error("Item ID=%u (image item): Advanced Profile requires AV1 High Profile for image items", av1C.first);

            if(!strcmp(sym.name, "seq_level_idx_0"))
              if(sym.value > 16 /*6*/)
                out->error("Item ID=%u (image item): Advanced Profile requires AV1 level 6 or lower for image items", av1C.first);
          }
        }

        // image sequences

        auto imageSequenceAv1Cs = getAv1CPerTrackId(root);

        for(auto& av1C : imageSequenceAv1Cs)
        {
          for(auto& sym : av1C.second->syms)
          {
            if(!strcmp(sym.name, "seq_profile"))
              if(sym.value > 1 /*Main or High*/)
                out->error("TrackId=%u (image sequence): Advanced Profile requires AV1 Main or High Profiles for image sequences", av1C.first);

            if(!strcmp(sym.name, "seq_level_idx_0"))
              if(sym.value > 13 /*5.1*/)
                out->error("TrackId=%u (image sequence): Advanced Profile requires AV1 level 5.1 or lower for image sequences", av1C.first);
          }
        }
      }
    },
    {
      "Section 5\n"
      "AVIF files shall contain either the 'avif', 'avis', or 'avio' brand\n",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "major_brand") || !strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("avif") || sym.value == FOURCC("avis") || sym.value == FOURCC("avio"))
                  found = true;

        if(!found)
          out->error("'avif', 'avis', or 'avio' brand not found");
      }
    }
  };
  return rulesProfiles;
}

