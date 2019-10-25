#include "spec.h"
#include "fourcc.h"
#include <algorithm> // find
#include <cstring>
#include <sstream>
#include <vector>
#include <map>

enum HEVC
{
  HEVC_MAIN = 0x01,
  HEVC_MAIN_STILL_PICTURE = 0x03
};

static std::map<int64_t, std::string> hevcProfiles {
  { 0x01, "Main" },
  { 0x02, "Main 10" },
  { 0x03, "Main Still Picture" },
};

static std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc)
{
  std::vector<const Box*> res;

  for(auto& box : root.children)
  {
    if(box.fourcc == fourcc)
    {
      res.push_back(&box);
    }
    else
    {
      auto b = findBoxes(box, fourcc);
      res.insert(res.end(), b.begin(), b.end());
    }
  }

  return res;
}

bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root)
{
  for(auto& rule : spec.rules)
  {
    std::stringstream ss(rule.caption);
    std::string line;
    std::getline(ss, line);
    std::stringstream ssl(line);
    std::string word;
    ssl >> word;

    if(word != "Section")
      throw std::runtime_error("Rule caption is misformed.");

    ssl >> word;

    if(word.rfind(section, 0) == 0)
    {
      struct Report : IReport
      {
        void error(const char*, ...) override
        {
          ++errorCount;
        }

        int errorCount = 0;
      };
      Report r;
      rule.check(root, &r);

      if(r.errorCount)
        return false;
    }
  }

  return true;
}

static bool usesBrand(Box const& root, uint32_t brandFourcc)
{
  for(auto& box : root.children)
    if(box.fourcc == FOURCC("ftyp"))
      for(auto& sym : box.syms)
        if(strcmp(sym.name, "compatible_brand"))
          if(sym.value == brandFourcc)
            return true;

  return false;
}

static void profileCommonChecks(const SpecDesc& spec, const char* profileName, Box const& root, IReport* out)
{
  if(!checkRuleSection(spec, "8.2", root))
    out->error("%s: self-containment (subclause 8.2) is not conform", profileName);

  if(!checkRuleSection(spec, "8.3", root))
    out->error("%s: single-layer (subclause 8.3) is not conform", profileName);

  if(!checkRuleSection(spec, "8.4", root))
    out->error("%s: grid-limit (subclause 8.4) is not conform", profileName);

  if(!checkRuleSection(spec, "8.5", root))
    out->error("%s: single-track (subclause 8.5) is not conform", profileName);

  if(!checkRuleSection(spec, "8.7", root))
    out->error("%s: matched-duration (subclause 8.7) is not conform", profileName);
}

static void checkAvcHevcLevel(IReport* out, const char* profileName, int rawLevel, double maxLevel)
{
  auto const level = (double)rawLevel / 30.0;

  if(level > maxLevel)
    out->error("%s: invalid level %g found, expecting %g or lower.", profileName, level, maxLevel);
}

static void checkHevcProfilesLevels(IReport* out, const char* profileName, std::vector<const Box*> hvcCs, std::vector<std::string> profiles, double maxLevel)
{
  for(auto& hvcc : hvcCs)
  {
    for(auto& sym : hvcc->syms)
    {
      if(!strcmp(sym.name, "general_profile_idc"))
      {
        bool found = false;

        if(hevcProfiles.find(sym.value) != hevcProfiles.end())
          for(auto& profile : profiles)
            if(hevcProfiles[sym.value] == profile)
              found = true;

        if(!found)
          out->error("%s: invalid profile 0x%llx found", profileName, sym.value);
      }
      else if(!strcmp(sym.name, "general_level_idc"))
      {
        checkAvcHevcLevel(out, profileName, sym.value, maxLevel);
      }
    }
  }
}

const std::initializer_list<RuleDesc> getRulesProfiles(const SpecDesc& spec)
{
  static const SpecDesc& globalSpec = spec;
  (void)globalSpec;
  static const
  std::initializer_list<RuleDesc> rulesProfiles =
  {
    {
      "Section A.3\n"
      "MIAF HEVC Basic profile\n"
      "Section A.3.1\n"
      "This profile includes the requirements of\n"
      "- self-containment (subclause 8.2),\n"
      "- single-layer (subclause 8.3),\n"
      "- grid-limit (subclause 8.4),\n"
      "- single-track (subclause 8.5),\n"
      "- matched-duration (subclause 8.7).\n"
      "Section A.3.2\n"
      "Images coded with the following profiles at Main tier may be present and shall\n"
      "be supported by the MIAF reader as coded image items; the level signalled by\n"
      "the file shall be the indicated level or lower:\n"
      "- HEVC Main Still Picture Profile, Level 6,\n"
      "- HEVC Main Profile, Level 6.\n"
      "NOTE: These profiles only support the 4:2:0 chroma sampling format and a bit\n"
      "      depth of 8 bits."
      "A.3.3 Image sequence and video coding\n"
      "HEVC image sequences shall be stored in accordance with ISO/IEC 14496-15.\n"
      "For image sequence tracks conforming to this MIAF profile, the requirements are\n"
      "the same as for image items in subclause A.3.2.\n"
      "For video tracks conforming to this MIAF profile, HEVC Main Profile at Main\n"
      "tier level 5.1 or lower shall be indicated in the sample entry and shall be\n"
      "supported by the MIAF reader.\n"
      "A.3.4 Brand identification\n"
      "The brand to identify files that conform to the MIAF HEVC basic profile is 'MiHB'.",
      [] (Box const& root, IReport* out)
      {
        if(!usesBrand(root, FOURCC("MiHB")))
          return;

        auto const profileName = "MIAF HEVC Basic profile ('MiHB')";

        profileCommonChecks(globalSpec, profileName, root, out);

        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
            checkHevcProfilesLevels(out, profileName, findBoxes(box, FOURCC("hvcC")), { "Main", "Main Still Picture" }, 6.0);

          if(box.fourcc == FOURCC("moov"))
            checkHevcProfilesLevels(out, profileName, findBoxes(box, FOURCC("hvcC")), { "Main", "Main Still Picture" }, 5.1);
        }
      }
    },
    {
      "Section A.4\n"
      "MIAF HEVC Advanced profile\n"
      "A.4.1 Adopted shared constraints\n"
      "This profile includes the requirements of\n"
      "- self-containment (subclause 8.2),\n"
      "- single-layer (subclause 8.3),\n"
      "- grid-limit (subclause 8.4),\n"
      "- single-track (subclause 8.5),\n"
      "- matched-duration (subclause 8.7).\n"
      "A.4.2 Image item coding\n"
      "Images conforming to the MIAF HEVC Basic profile or coded with the following\n"
      "HEVC profiles at Main tier may be present and shall be supported by the MIAF\n"
      "reader and MIAF renderer; the level signalled by the file shall be the\n"
      "indicated level or lower:\n"
      "- Main 10, Level 6,\n"
      "- Main 10 Intra, Level 6,\n"
      "- Main Intra, Level 6,\n"
      "- Main 10 Still Picture, Level 6,\n"
      "- Main 4:2:2 10 Intra, Level 6.\n"
      "A.4.3 Image sequence and video coding\n"
      "For image sequence tracks conforming to this MIAF profile, the requirements are\n"
      "the same as for image items in subclause A.4.2.\n"
      "For video tracks conforming to this MIAF profile, the requirements of the MIAF\n"
      "HEVC Basic profile apply or HEVC Main 10 or Main 4:2:2 10 profile at Main tier\n"
      "level 5.1 or lower shall be indicated in the sample entry and shall be\n"
      "supported by the MIAF reader.\n"
      "A.4.4 Brand identification\n"
      "The brand to identify files that conform to the MIAF HEVC advanced profile is\n"
      "'MiHA'.",
      [] (Box const& root, IReport* out)
      {
        if(!usesBrand(root, FOURCC("MiHA")))
          return;

        auto const profileName = "MIAF HEVC Advanced profile ('MiHA')";

        profileCommonChecks(globalSpec, profileName, root, out);

        // compliance to Basic profile suffices
        if(checkRuleSection(globalSpec, "A.3", root))
          return;

        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
            checkHevcProfilesLevels(out, profileName, findBoxes(box, FOURCC("hvcC")), { "Main 10", "Main 10 Intra", "Main Intra", "Main 10 Still Picture", "Main 4:2:2 10 Intra" }, 6.0);

          if(box.fourcc == FOURCC("moov"))
            checkHevcProfilesLevels(out, profileName, findBoxes(box, FOURCC("hvcC")), { "Main 10", "Main 4:2:2 10" }, 5.1);
        }
      }
    },
    {
      "Section A.5\n"
      "MIAF HEVC Extended profile\n"
      "A.5.1 Adopted shared constraints\n"
      "This profile includes the requirements of\n"
      "- self-containment (subclause 8.2),\n"
      "- single-layer (subclause 8.3),\n"
      "- grid-limit (subclause 8.4),\n"
      "- single-track (subclause 8.5),\n"
      "- matched-duration (subclause 8.7).\n"
      "A.5.2 Image item coding\n"
      "Images conforming to the MIAF HEVC basic profile or MIAF HEVC advanced profile or coded with the following HEVC profiles at Main tier may be present and shall be supported by the MIAF reader and MIAF renderer; the level signalled by the file shall be the indicated level or lower:\n"
      "- Main 4:4:4 10, Level 6,\n"
      "- Main 4:4:4 Still Picture, Level 6,\n"
      "- Main 4:4:4 10 Intra, Level 6,\n"
      "- Main 4:4:4, Level 6,\n"
      "- Monochrome 10, Level 6,\n"
      "- Monochrome, Level 6.\n"
      "A.5.3 Image sequence and video coding\n"
      "For image sequence tracks conforming to this MIAF profile, the requirements are the same as for image items in subclause A.5.2.\n"
      "For video tracks conforming to this MIAF profile, the requirements of the MIAF HEVC advanced Profile apply or HEVC Main 4:4:4 10 profile at Main tier level 5.1 or lower shall be indicated in the sample entry and shall be supported by the MIAF reader.\n"
      "A.5.4 Brand identification\n"
      "The brand to identify files that conform to the MIAF HEVC extended profile is 'MiHE'.",
      [] (Box const& root, IReport* out)
      {
        if(!usesBrand(root, FOURCC("MiHE")))
          return;

        auto const profileName = "MIAF HEVC Extended profile ('MiHE')";

        profileCommonChecks(globalSpec, profileName, root, out);

        // compliance to Basic or Advanced profiles suffice
        if(checkRuleSection(globalSpec, "A.3", root) || checkRuleSection(globalSpec, "A.4", root))
          return;

#if 0 // TODO
        "A.5.2 Image item coding\n"
        "Images conforming to the MIAF HEVC basic profile or MIAF HEVC advanced profile"
        "or"
        "- Main 4:4:4 10, Level 6,\n"
        "- Main 4:4:4 Still Picture, Level 6,\n"
        "- Main 4:4:4 10 Intra, Level 6,\n"
        "- Main 4:4:4, Level 6,\n"
        "- Monochrome 10, Level 6,\n"
        "- Monochrome, Level 6.\n"
        "A.5.3 Image sequence and video coding\n"
        "For video tracks, requirements of the MIAF HEVC advanced Profile apply or HEVC Main 4:4:4 10 profile at Main tier level 5.1"
#endif
      }
    },
    {
      "Section A.6\n"
      "A.6 MIAF AVC Basic profile\n"
      "A.6.1 Adopted shared constraints\n"
      "This profile includes the requirements of\n"
      "- self-containment (subclause 8.2),\n"
      "- single-layer (subclause 8.3),\n"
      "- grid-limit (subclause 8.4),\n"
      "- single-track (subclause 8.5),\n"
      "- matched-duration (subclause 8.7).\n"
      "A.6.2 Image item coding\n"
      "Images coded with the following profiles may be present and shall be supported by the MIAF reader as coded image items; the level signalled by the file shall be the indicated level or lower:\n"
      "- AVC Progressive High Profile, Level 5.2,\n"
      "- AVC Constrained High Profile, Level 5.2.\n"
      "A.6.3 Image sequence and video coding\n"
      "AVC image sequences shall be stored in accordance with ISO/IEC 14496-15.\n"
      "For image sequence tracks conforming to this MIAF profile, the requirements are the same as for image items in subclause A.6.2.\n"
      "For video tracks conforming to this MIAF profile, AVC High Profile level 5.1 or lower shall be indicated in the sample entry and shall be supported by the MIAF reader.\n"
      "A.6.4 Brand identification\n"
      "The brand to identify files that conform to the MIAF AVC Basic profile is 'MiAB'.",
      [] (Box const& root, IReport* out)
      {
        if(!usesBrand(root, FOURCC("MiAB")))
          return;

        auto const profileName = "MIAF AVC Basic profile ('MiAB')";

        profileCommonChecks(globalSpec, profileName, root, out);

        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
          {
            auto avcCs = findBoxes(box, FOURCC("avcC"));

            for(auto& avcc : avcCs)
            {
              for(auto& sym : avcc->syms)
              {
                if(!strcmp(sym.name, "AVCProfileIndication"))
                {
                  if(sym.value > 100)
                    out->error("%s: profile_idc (0x%llx) is higher than 100)", profileName, sym.value);
                }
                else if(!strcmp(sym.name, "AVCLevelIndication"))
                {
                  checkAvcHevcLevel(out, profileName, sym.value, 5.2);
                }
              }
            }
          }

          if(box.fourcc == FOURCC("moov"))
          {
            auto avcCs = findBoxes(box, FOURCC("avcC"));

            for(auto& avcc : avcCs)
            {
              for(auto& sym : avcc->syms)
              {
                if(!strcmp(sym.name, "AVCProfileIndication"))
                {
                  if(sym.value > 100)
                    out->error("%s: profile_idc (0x%llx) is higher than 100)", profileName, sym.value);
                }
                else if(!strcmp(sym.name, "AVCLevelIndication"))
                {
                  checkAvcHevcLevel(out, profileName, sym.value, 5.1);
                }
              }
            }
          }
        }
      }
    },
#if 0 // enable when codec rules are implemented
    {
      "Section 7.2.1.2\n"
      "Files shall also carry a compatible brand to identify the MIAF profile to which\n"
      "the file conforms, as defined in Annex A or external specifications.",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> compatibleBrands;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(strcmp(sym.name, "compatible_brand"))
                compatibleBrands.push_back(sym.value);

        if(checkRuleSection(globalSpec, "A.3", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiHB")) == compatibleBrands.end())
          out->error("File conforms to 'MiHB' brand but 'MiHB' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "A.4", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiHA")) == compatibleBrands.end())
          out->error("File conforms to 'MiHA' brand but 'MiHA' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "A.5", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiHE")) == compatibleBrands.end())
          out->error("File conforms to 'MiHE' brand but 'MiHE' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "A.6", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiAB")) == compatibleBrands.end())
          out->error("File conforms to 'MiAB' brand but 'MiAB' is not in the 'ftyp' compatible_brand list");
      }
    }
#endif
  };

  return rulesProfiles;
}

