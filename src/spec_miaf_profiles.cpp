#include "spec.h"
#include "fourcc.h"
#include <cstring>
#include <sstream>

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

static void profileCommonChecks(const SpecDesc& spec, const char* profileName, uint32_t brandFourcc, Box const& root, IReport* out)
{
  bool found = false;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("ftyp"))
      for(auto& sym : box.syms)
        if(strcmp(sym.name, "compatible_brand"))
          if(sym.value == brandFourcc)
            found = true;

  if(!found)
    return;

  if(!checkRuleSection(spec, "8.2", root))
    out->error("%s: self-containment (subclause 8.2) is not conform.", profileName);

  if(!checkRuleSection(spec, "8.3", root))
    out->error("%s: single-layer (subclause 8.3) is not conform.", profileName);

  if(!checkRuleSection(spec, "8.4", root))
    out->error("%s: grid-limit (subclause 8.4) is not conform.", profileName);

  if(!checkRuleSection(spec, "8.5", root))
    out->error("%s: single-track (subclause 8.5) is not conform.", profileName);

  if(!checkRuleSection(spec, "8.7", root))
    out->error("%s: matched-duration (subclause 8.7) is not conform.", profileName);
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
        profileCommonChecks(globalSpec, "MIAF HEVC Basic profile", FOURCC("MiHB"), root, out);

#if 0 // TODO
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
#endif
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
        profileCommonChecks(globalSpec, "MIAF HEVC Advanced profile", FOURCC("MiHA"), root, out);

#if 0 // TODO
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
#endif
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
        profileCommonChecks(globalSpec, "MIAF HEVC Extended profile", FOURCC("MiHE"), root, out);

#if 0 // TODO
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
        profileCommonChecks(globalSpec, "MIAF AVC Basic profile", FOURCC("MiAB"), root, out);

#if 0 // TODO
        "Images coded with the following profiles may be present and shall be supported by the MIAF reader as coded image items; the level signalled by the file shall be the indicated level or lower:\n"
        "- AVC Progressive High Profile, Level 5.2,\n"
        "- AVC Constrained High Profile, Level 5.2.\n"
        "A.6.3 Image sequence and video coding\n"
        "For video tracks conforming to this MIAF profile, AVC High Profile level 5.1 or lower shall be indicated in the sample entry and shall be supported by the MIAF reader.\n"
#endif
      }
    },
  };

  return rulesProfiles;
}

