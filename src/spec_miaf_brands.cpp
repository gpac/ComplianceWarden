#include "spec.h"
#include "fourcc.h"
#include <algorithm> // std::find
#include <cstring>
#include <map>

bool isVisualSampleEntry(uint32_t fourcc);

bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root);

const std::initializer_list<RuleDesc> getRulesBrands(const SpecDesc& spec)
{
  static const SpecDesc& globalSpec = spec;
  static const
  std::initializer_list<RuleDesc> rulesBrands =
  {
    {
      "Section 10.1\n"
      "A MIAF file shall use the filename extensions specified by HEIF to identify the\n"
      "presence of specific image coding formats",
      [] (Box const& root, IReport* out)
      {
        std::string filename;

        for(auto& field : root.syms)
          if(!strcmp(field.name, "filename"))
            filename.push_back((char)field.value);

        auto extPos = filename.find_last_of('.');

        if(extPos > filename.length())
        {
          out->error("Filename \"%s\" has no extension", filename.c_str());
          extPos = filename.length() - 1;
        }

        auto const ext = filename.substr(extPos + 1);

        auto getExtensionFromBrand = [] (Box const& root) -> std::vector<const char*> {
            for(auto& box : root.children)
              if(box.fourcc == FOURCC("ftyp"))
                for(auto& sym : box.syms)
                  if(!strcmp(sym.name, "compatible_brand"))
                    switch(sym.value)
                    {
                    case FOURCC("avci"):
                      return { "avci" };
                    case FOURCC("avcs"):
                      return { "avcs" };
                    case FOURCC("heic"):
                    case FOURCC("heix"):
                    case FOURCC("heim"):
                    case FOURCC("heis"):
                      return { "heic", "hif" };
                    case FOURCC("hevc"):
                    case FOURCC("hevx"):
                    case FOURCC("hevm"):
                    case FOURCC("hevs"):
                      return { "heics", "hif" };
                    }

            auto isSequence = [] (Box const& root) -> bool {
                for(auto& box : root.children)
                  if(box.fourcc == FOURCC("moov"))
                    for(auto& moovChild : box.children)
                      if(moovChild.fourcc == FOURCC("trak"))
                        return true;

                return false;
              };

            if(isSequence(root))
              return { "heifs", "hif" };
            else
              return { "heif", "hif" };
          };

        bool found = false;
        auto possibleExts = getExtensionFromBrand(root);

        for(auto e : possibleExts)
          if(ext == e)
            found = true;

        std::string expected;

        for(auto e : possibleExts)
        {
          expected += e;
          expected += " ";
        }

        if(!found)
          out->error("File extension for \"%s\" doesn't match: expecting one of '%s', got '%s'", filename.c_str(), expected.c_str(), ext.c_str());
      }
    },
    {
      "Section 10.2\n"
      "'MiPr' in the compatible_brands in the FileTypeBox specifies that a file\n"
      "conforming to 'miaf' brand also conforms to the following constraints:\n"
      "- The MetaBox shall follow the FileTypeBox. There shall be no intervening boxes\n"
      "  between the FileTypeBox and the MetaBox except at most one BoxFileIndexBox.\n"
      "- The MediaDataBox shall not occur before the MetaBox.\n"
      "- At most one top-level FreeSpaceBox is allowed, which, if present, shall be\n"
      "  between the MetaBox and the MediaDataBox. There shall be no other top-level\n"
      "  FreeSpaceBox in the file.\n"
      "- The primary image item conforms to a MIAF profile.\n"
      "- There is at least one MIAF thumbnail image item present for the primary image\n"
      "  item and the coded data for the thumbnail image items precede in file order\n"
      "  the coded data for the primary item.\n"
      "- The maximum number of bytes from the beginning of the file to the last byte\n"
      "  of the coded data for at least one of the thumbnail images of the primary\n"
      "  item, or the primary item itself, is 128 000 bytes.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MiPr"))
                  found = true;

        if(!found)
          return;

        auto boxOrderCheck = [&] () -> bool {
            if(root.children.size() < 2)
              return false;

            if(root.children[0].fourcc != FOURCC("ftyp"))
              return false;

            if(root.children[1].fourcc == FOURCC("meta"))
              return true;
            else if(root.children.size() >= 3
                    && root.children[1].fourcc == FOURCC("fidx") && root.children[2].fourcc == FOURCC("meta"))
              return true;
            else
              return false;
          };

        if(!boxOrderCheck())
          out->error("'MiPr' brand: the MetaBox shall follow the FileTypeBox (with at most one intervening BoxFileIndexBox)");

        for(auto& b : root.children)
        {
          if(b.fourcc == FOURCC("mdat"))
            out->error("'MiPr' brand: the MediaDataBox shall not occur before the MetaBox");

          if(b.fourcc == FOURCC("meta"))
            break;
        }

        int numFreeSpaceBox = 0;
        bool seenMeta = false;

        for(auto& b : root.children)
        {
          if(b.fourcc == FOURCC("mdat"))
            break;

          if(b.fourcc == FOURCC("meta"))
            seenMeta = true;

          if(b.fourcc == FOURCC("free"))
          {
            numFreeSpaceBox++;

            if(!seenMeta)
              out->error("'MiPr' brand: top-level FreeSpaceBox shall be between the MetaBox and the MediaDataBox");
          }
        }

        if(numFreeSpaceBox > 1)
          out->error("'MiPr' brand: at most one top-level FreeSpaceBox is allowed");

        // The primary image item conforms to a MIAF profile.
        if(!checkRuleSection(globalSpec, "7.", root))
          out->error("'MiPr' brand: this file shall conform to MIAF (Section 7)");

        if(!checkRuleSection(globalSpec, "8.", root))
          out->error("'MiPr' brand: this file shall conform to MIAF (Section 8)");

        found = false;
        uint32_t primaryItemId = -1;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("pitm"))
              {
                found = true;

                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "item_ID"))
                    primaryItemId = field.value;
              }

        if(!found)
          out->error("'MiPr' brand: PrimaryItemBox is required");

        std::map<uint32_t /*itemId*/, int64_t /*offset*/> pitmSelfAndThmbs;
        pitmSelfAndThmbs.insert({ primaryItemId, -1 }); // self

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iref"))
                for(auto& field : metaChild.syms)
                {
                  if(!strcmp(field.name, "box_type"))
                    if(field.value != FOURCC("thmb"))
                      continue;

                  if(!strcmp(field.name, "to_item_ID"))
                  {
                    if(field.value != primaryItemId)
                      for(auto& sym : metaChild.syms)
                        if(!strcmp(sym.name, "from_item_ID"))
                          pitmSelfAndThmbs.insert({ sym.value, -1 });
                  }
                }

        if(pitmSelfAndThmbs.empty())
          out->error("'MiPr' brand: there is at least one MIAF thumbnail image item present for the primary image item");

        int64_t pitmFirstOffset = -1;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iloc"))
              {
                // for MIAF coded image items construction_method==0 and data_reference_index==0
                uint32_t itemId = 0;

                for(auto& sym : metaChild.syms)
                {
                  if(!strcmp(sym.name, "item_ID"))
                    itemId = sym.value;

                  if(pitmSelfAndThmbs.find(itemId) != pitmSelfAndThmbs.end())
                  {
                    if(itemId == primaryItemId)
                      if(!strcmp(sym.name, "base_offset"))
                        pitmFirstOffset = sym.value;

                    if(!strcmp(sym.name, "base_offset") || !strcmp(sym.name, "extent_offset"))
                      pitmSelfAndThmbs[itemId] = sym.value;

                    if(!strcmp(sym.name, "base_offset") || !strcmp(sym.name, "extent_offset"))
                      pitmSelfAndThmbs[itemId] += sym.value;
                  }
                }
              }

        found = false;

        for(auto& item : pitmSelfAndThmbs)
        {
          if(item.second < pitmSelfAndThmbs[primaryItemId])
            out->error("'MiPr' brand: coded data offset thumbnail (item_ID=%u offset=%lld) precedes coded data for the primary item (item_ID=%u offset=%lld)",
                       item.first, item.second, primaryItemId, pitmFirstOffset);

          if(item.second < 128000)
            found = true;
        }

        if(!found)
          out->error("'MiPr' brand: The maximum number of bytes from the beginning of the file to\n"
                     "  the last byte of the coded data for at least one of the thumbnail images of the\n"
                     "  primary item, or the primary item itself, is 128 000 bytes.");
      }
    },
    {
      "Section 10.3\n"
      "The presence of the animation MIAF application brand indication ('MiAn') in the\n"
      "FileTypeBox indicates that the file conforms to the following additional\n"
      "constraints:\n"
      "- There shall be:\n"
      "  * exactly one non-auxiliary video track or non-auxiliary image sequence track\n"
      "  * at most one auxiliary video track (which shall be an alpha plane track,\n"
      "    when present),\n"
      "  * at most one audio track, and\n"
      "  * no other media tracks.\n"
      "- The luma sample rate of each video track shall be less than or equal to\n"
      "  62 914 560 samples per second.\n"
      "- The constraints of subclause 8.6 apply.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MiAn"))
                  found = true;

        if(!found)
          return;

        {
          std::vector<uint32_t> trackHandlers;
          bool foundAlphaTrack = false;

          for(auto& box : root.children)
            if(box.fourcc == FOURCC("moov"))
              for(auto& moovChild : box.children)
                if(moovChild.fourcc == FOURCC("trak"))
                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("mdia"))
                    {
                      for(auto& mdiaChild : trakChild.children)
                        if(mdiaChild.fourcc == FOURCC("minf"))
                        {
                          for(auto& minfChild : mdiaChild.children)
                            if(minfChild.fourcc == FOURCC("stbl"))
                              for(auto& stblChild : minfChild.children)
                                if(stblChild.fourcc == FOURCC("stsd"))
                                  for(auto& stsdChild : stblChild.children)
                                    if(isVisualSampleEntry(stsdChild.fourcc))
                                      for(auto& sampleEntryChild : stsdChild.children)
                                        if(sampleEntryChild.fourcc == FOURCC("auxi"))
                                        {
                                          std::string auxType;

                                          for(auto& sym : sampleEntryChild.syms)
                                            if(!strcmp(sym.name, "aux_track_type"))
                                            {
                                              auxType.push_back((char)sym.value);

                                              if(auxType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")
                                                foundAlphaTrack = true;
                                            }
                                        }
                        }
                        else if(mdiaChild.fourcc == FOURCC("hdlr"))
                          for(auto& sym : mdiaChild.syms)
                            if(!strcmp(sym.name, "handler_type"))
                              trackHandlers.push_back(sym.value);
                    }

          int numVideoTracks = 0, numAudioTracks = 0, numAuxTracks = 0;

          for(auto hdlr : trackHandlers)
          {
            switch(hdlr)
            {
            case FOURCC("vide"):
            case FOURCC("pict"):
              numVideoTracks++;
              break;
            case FOURCC("soun"):
              numAudioTracks++;
              break;
            case FOURCC("auxv"):
              numAuxTracks++;
              break;
            default:
              out->error("'MiAn' brand: no other media tracks than video/image sequence, audio, and auxiliary allowed. Found \"%s\"", toString(hdlr).c_str());
              break;
            }

            if(numVideoTracks != 1)
              out->error("'MiAn' brand: there shall be exactly one non-auxiliary video track or non-auxiliary image sequence track, found %d", numVideoTracks);

            if(numAudioTracks > 1)
              out->error("'MiAn' brand: at most one audio track, found %d", numAudioTracks);

            if(numAuxTracks > 1 || !foundAlphaTrack)
              out->error("'MiAn' brand: there shall be at most one auxiliary video track 'auxv' (found %d) (which shall be an alpha plane track: \"%s\")",
                         numAuxTracks, foundAlphaTrack ? "true" : "false");
          }

          if(!checkRuleSection(globalSpec, "10.6", root))
            out->error(" 'MiAn' brand: this file shall conform to the 'MiCm' brand");
        }
      }
    },
    {
      "Section 10.4\n"
      "A track indicated to conform to this brand shall be constrained as follows:\n"
      "- The track shall be an image sequence ('pict') track.\n"
      "- In the image sequence track, any single coded picture shall be decodable by\n"
      "  decoding a maximum of two coded pictures (i.e. the picture itself and at most\n"
      "  one reference), and these two coded pictures shall be a valid bitstream.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MiBu"))
                  found = true;

        if(!found)
          return;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("hdlr"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "handler_type"))
                    if(field.value != FOURCC("pict"))
                      out->error("'MiBu' brand: the track shall be an image sequence ('pict') track");
      }
    },
    {
      "Section 10.5\n"
      "The presence of the brand 'MiAC' in the FileTypeBox indicates that the file\n"
      "conforms to the following additional constraints:\n"
      " - It conforms to the constraints of both the 'MiCm' and the 'MiAn' brands,\n"
      "   with the following constraints:\n"
      " - There is exactly one auxiliary alpha video track.\n"
      " - The non-auxiliary video track uses the 'vide' handler, and is not\n"
      "   pre-multiplied.\n"
      " - The tracks are fragmented.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MiAC"))
                  found = true;

        if(!found)
          return;

        if(!checkRuleSection(globalSpec, "10.6", root))
          out->error("'MiAC' brand: this file shall conform to the 'MiCm' brand");

        if(!checkRuleSection(globalSpec, "10.3", root))
          out->error("'MiAC' brand: this file shall conform to the 'MiAn' brand");

        {
          int alphaTrackNum = 0;

          for(auto& box : root.children)
            if(box.fourcc == FOURCC("moov"))
              for(auto& moovChild : box.children)
                if(moovChild.fourcc == FOURCC("trak"))
                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("mdia"))
                      for(auto& mdiaChild : trakChild.children)
                        if(mdiaChild.fourcc == FOURCC("minf"))
                          for(auto& minfChild : mdiaChild.children)
                            if(minfChild.fourcc == FOURCC("stbl"))
                              for(auto& stblChild : minfChild.children)
                                if(stblChild.fourcc == FOURCC("stsd"))
                                  for(auto& stsdChild : stblChild.children)
                                    if(isVisualSampleEntry(stsdChild.fourcc))
                                      for(auto& sampleEntryChild : stsdChild.children)
                                        if(sampleEntryChild.fourcc == FOURCC("auxi"))
                                        {
                                          std::string auxType;

                                          for(auto& sym : sampleEntryChild.syms)
                                            if(!strcmp(sym.name, "aux_track_type"))
                                            {
                                              auxType.push_back((char)sym.value);

                                              if(auxType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")
                                                alphaTrackNum++;
                                            }
                                        }

          if(alphaTrackNum != 1)
            out->error("'MiAC' brand: there shall be exactly one auxiliary alpha video track, found %d.", alphaTrackNum);
        }

        {
          std::vector<uint32_t /*hdlr*/> trackHandlers;

          for(auto& box : root.children)
            if(box.fourcc == FOURCC("moov"))
              for(auto& moovChild : box.children)
                if(moovChild.fourcc == FOURCC("trak"))
                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("mdia"))
                      for(auto& mdiaChild : trakChild.children)
                        if(mdiaChild.fourcc == FOURCC("hdlr"))
                          for(auto& sym : mdiaChild.syms)
                            if(!strcmp(sym.name, "handler_type"))
                              trackHandlers.push_back((uint32_t)sym.value);

          if(trackHandlers.size() != 2)
            out->error("'MiAC' brand: \"the non-auxiliary video track shall use the 'vide' handler\" implies 2 tracks but %d were found", (int)trackHandlers.size());

          if(!(trackHandlers[0] == FOURCC("auxv") && trackHandlers[1] == FOURCC("vide")) && !(trackHandlers[0] == FOURCC("vide") && trackHandlers[1] == FOURCC("auxv")))
            out->error("'MiAC' brand: the non-auxiliary video track shall use the 'vide' handler, found handlers '%s' and '%s'", toString(trackHandlers[0]).c_str(), toString(trackHandlers[1]).c_str());
        }

        found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("mvex"))
                found = true;

        if(!found)
          out->error("'MiAC' brand: the tracks are fragmented");
      }
    },
    {
      "Section 10.6\n"
      "The presence of the brand 'MiCm' in the FileTypeBox indicates that the file\n"
      "contains movie fragments that conform to the constraints of the 'cmfc' brand of\n"
      "ISO/IEC 23000-19, and the following additional constraints that apply when a\n"
      "MIAF file contains multiple tracks (e.g. a video or image sequence track and an\n"
      "auxiliary track):\n"
      " - each track, if considered separately, shall be a conforming CMAF track as\n"
      "   defined in ISO/IEC 23000-19. In other words, if all boxes related to the\n"
      "   other tracks were removed (e.g. file-level boxes such as MovieFragmentBoxes,\n"
      "   and boxes in the MovieBox such as the TrackBox or the TrackExtendsBox), the\n"
      "   content shall be conforming to the brand 'cmfc' defined in ISO/IEC 23000-19;\n"
      " - the set of CMAF tracks associated with all MIAF tracks (including any audio)\n"
      "   shall be of the same duration, within a tolerance of the longest CMAF\n"
      "   fragment duration of any CMAF track;\n"
      " - the set of CMAF tracks associated with the MIAF visual tracks shall have the\n"
      "   same duration, same number of fragments and fragments shall be time-aligned.\n"
      "   Fragments of the different CMAF tracks shall also be interleaved in the MIAF\n"
      "   file.",
      [] (Box const& root, IReport* /*out*/)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("MiCm"))
                  found = true;

        if(!found)
          return;

        // TODO: CMAF
      }
    },
#if 0 // enable when confirmed this is what we want (changes all the rules and samples)
    {
      "Section 7.2.1.2\n"
      "The FileTypeBox shall contain, in the compatible_brands list [...] brand(s)\n"
      "identifying conformance to this document (specified in Clause 10).",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> compatibleBrands;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                compatibleBrands.push_back(sym.value);

        if(checkRuleSection(globalSpec, "10.2", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiPr")) == compatibleBrands.end())
          out->error("File conforms to 'MiPr' brand but 'MiPr' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "10.3", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiAn")) == compatibleBrands.end())
          out->error("File conforms to 'MiAn' brand but 'MiAn' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "10.4", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiBu")) == compatibleBrands.end())
          out->error("File conforms to 'MiBu' brand but 'MiBu' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "10.5", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiAC")) == compatibleBrands.end())
          out->error("File conforms to 'MiAC' brand but 'MiAC' is not in the 'ftyp' compatible_brand list");

        if(!checkRuleSection(globalSpec, "10.6", root) && std::find(compatibleBrands.begin(), compatibleBrands.end(), FOURCC("MiCm")) == compatibleBrands.end())
          out->error("File conforms to 'MiCm' brand but 'MiCm' is not in the 'ftyp' compatible_brand list");
      }
    }
#endif
  };
  return rulesBrands;
}

