#include "../../utils/av1_utils.h"
#include "../../utils/box_reader_impl.h"
#include "../../utils/get_data.h"

#include <algorithm>
#include <cstring> // strcmp
#include <map>
#include <string>

bool checkRuleSection(const SpecDesc &spec, const char *section, Box const &root);
std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc);

std::vector<uint32_t> getAv1Tracks(const Box &root)
{

  std::vector<uint32_t> res;

  auto trakBoxes = findBoxes(root, FOURCC("trak"));
  for(auto &trakBox : trakBoxes) {

    auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
    if(tkhdBoxes.size() != 1) {
      // SKIP TRACK
      continue;
    }

    uint32_t thisTrackId = 0;
    for(auto &sym : tkhdBoxes[0]->syms)
      if(!strcmp(sym.name, "track_ID"))
        thisTrackId = sym.value;

    if(findBoxes(root, FOURCC("av01")).size() == 1) {
      res.push_back(thisTrackId);
    }
  }
  return res;
}

namespace
{

struct ResolutionDetails {
  bool valid{ false };
  int64_t width{ 0 };
  int64_t height{ 0 };
};

ResolutionDetails getOBUDetails(Box const &root, IReport *out)
{
  bool foundAv1 = false;

  auto stbls = findBoxes(root, FOURCC("stbl"));
  for(auto stbl : stbls) {
    for(auto &stblChild : stbl->children)
      if(stblChild.fourcc == FOURCC("stsd"))
        for(auto &stsdChild : stblChild.children)
          if(stsdChild.fourcc == FOURCC("av01"))
            for(auto &sampleEntryChild : stsdChild.children)
              if(sampleEntryChild.fourcc == FOURCC("av1C"))
                foundAv1 = true;

    if(!foundAv1)
      continue;

    for(auto &stblChild : stbl->children)
      if(stblChild.fourcc == FOURCC("stco") || stblChild.fourcc == FOURCC("co64")) {
        for(auto &sym : stblChild.syms)
          if(!strcmp(sym.name, "chunk_offset")) {
            BoxReader br;
            auto const probeSize = 1024;
            br.br = BitReader{ root.original + sym.value, probeSize };

            auto obuType = 0;
            Av1State stateUnused;
            while(obuType != OBU_SEQUENCE_HEADER) {
              obuType = parseAv1Obus(&br, stateUnused, false);
            }

            if(obuType != OBU_SEQUENCE_HEADER) {
              out->error("No OBU Sequence header found");
              return { false };
            }

            auto obuWidth = 0;
            auto obuHeight = 0;

            for(auto &sym : br.myBox.syms) {
              if(std::string(sym.name) == "max_frame_width_minus_1")
                obuWidth = sym.value + 1;
              else if(std::string(sym.name) == "max_frame_height_minus_1")
                obuHeight = sym.value + 1;
            }
            return { true, obuWidth, obuHeight };
          }
      }
  }

  return { false };
}

ResolutionDetails getAv01Details(Box const &root)
{
  auto av01Boxes = findBoxes(root, FOURCC("av01"));

  for(auto &it : av01Boxes) {
    auto av01Width = 0;
    auto av01Height = 0;
    for(auto &sym : it->syms) {
      if(std::string(sym.name) == "width") {
        av01Width = sym.value;
      }
      if(std::string(sym.name) == "height") {
        av01Height = sym.value;
      }
    }

    return { true, av01Width, av01Height };
  }
  return { false };
}

bool parseAv1Configs(
  Box const &root, IReport *out, uint32_t trackId, const Box &minfChild, Av1State &bsState, Av1State &av1cState,
  AV1CodecConfigurationRecord &av1cRef, bool displayParsingErrors, bool obuSizeShallBeOne, bool timingInfoShoudBeZero)
{
  bool av1BsFound = false, av1cFound = false, configOBUsFound = false;

  for(auto &stblChild : minfChild.children) {
    if(stblChild.fourcc == FOURCC("stsd")) {
      for(auto &stsdChild : stblChild.children)
        if(stsdChild.fourcc == FOURCC("av01"))
          for(auto &sampleEntryChild : stsdChild.children)
            if(sampleEntryChild.fourcc == FOURCC("av1C")) {
              for(auto &sym : sampleEntryChild.syms) {
                if(!strcmp(sym.name, "seq_profile")) {
                  if(!av1cFound) {
                    av1cRef.seq_profile = sym.value;
                    av1cFound = true;
                  } else if(!configOBUsFound) {
                    av1cState.av1c.seq_profile = sym.value;
                    configOBUsFound = true;
                  } else {
                    if(displayParsingErrors)
                      out->error("The configOBUs field SHALL contain at most one Sequence Header OBU. Found several.");
                    break; // exit after first seqHdr
                  }
                } else if(!strncmp(sym.name, "seq_level_idx", 13)) {
                  if(!configOBUsFound)
                    av1cRef.seq_level_idx_0 = sym.value;
                  else
                    av1cState.av1c.seq_level_idx_0 = sym.value;
                } else if(!strncmp(sym.name, "seq_tier", 8)) {
                  if(!configOBUsFound)
                    av1cRef.seq_tier_0 = sym.value;
                  else
                    av1cState.av1c.seq_tier_0 = sym.value;
                } else if(!strcmp(sym.name, "high_bitdepth")) {
                  if(!configOBUsFound)
                    av1cRef.high_bitdepth = sym.value;
                  else
                    av1cState.av1c.high_bitdepth = sym.value;
                } else if(!strcmp(sym.name, "twelve_bit")) {
                  if(!configOBUsFound)
                    av1cRef.twelve_bit = sym.value;
                  else
                    av1cState.av1c.twelve_bit = sym.value;
                } else if(!strcmp(sym.name, "mono_chrome")) {
                  if(!configOBUsFound)
                    av1cRef.mono_chrome = sym.value;
                  else
                    av1cState.av1c.mono_chrome = sym.value;
                } else if(!strcmp(sym.name, "chroma_subsampling_x")) {
                  if(!configOBUsFound)
                    av1cRef.chroma_subsampling_x = sym.value;
                  else
                    av1cState.av1c.chroma_subsampling_x = sym.value;
                } else if(!strcmp(sym.name, "chroma_subsampling_y")) {
                  if(!configOBUsFound)
                    av1cRef.chroma_subsampling_y = sym.value;
                  else
                    av1cState.av1c.chroma_subsampling_y = sym.value;
                } else if(!strcmp(sym.name, "chroma_sample_position")) {
                  if(!configOBUsFound)
                    av1cRef.chroma_sample_position = sym.value;
                  else
                    av1cState.av1c.chroma_sample_position = sym.value;
                } else if(!strcmp(sym.name, "obu_has_size_field")) {
                  if(obuSizeShallBeOne && sym.value != 1)
                    out->error("The flag obu_has_size_field SHALL be set to 1");
                } else if(!strcmp(sym.name, "timing_info_present_flag")) {
                  if(timingInfoShoudBeZero && sym.value != 0)
                    out->warning("timing_info_present_flag should be zero");
                }
              }
            }
    }
  }

  auto trakBoxes = findBoxes(root, FOURCC("trak"));
  for(auto &trakBox : trakBoxes) {
    auto av01Details = getAv01Details(*trakBox);
    if(!av01Details.valid) {
      continue;
    }

    // Get the track ID
    uint32_t trackId = 0;
    auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
    for(auto &tkhd : tkhdBoxes) {
      for(auto &sym : tkhd->syms)
        if(!strcmp(sym.name, "track_ID"))
          trackId = sym.value;
    }

    auto const samples = getData(root, out, trackId);
    if(samples.empty()) {
      out->warning("No sample found for trackId=%u", trackId);
    }

    for(auto i = 0u; i < samples.size(); i++) {
      BoxReader br;
      br.br = BitReader{ samples[i].position, (int)samples[i].size };

      while(!br.empty()) {
        auto type = parseAv1Obus(&br, bsState, false);

        if(type == OBU_SEQUENCE_HEADER) {
          av1BsFound = true;
          break;
        }
      }

      if(av1BsFound) {
        break;
      }
    }
  }

  if(av1cFound) {
    if(!av1BsFound)
      out->error(
        "[TrackId=%u] AV1 configuration should be present. Found in av1C(%d), in configOBUs(%d), in mdat(%d).", trackId,
        av1cFound, configOBUsFound, av1BsFound);

    if(!configOBUsFound)
      av1cState.av1c = av1cRef;

    out->covered();
    return true;
  }

  return false;
}

struct SampleGroup {
  uint32_t grouping_type;
  uint32_t sample_description_index;
};
struct SampleToGroup : std::vector<SampleGroup> {
};

std::vector<SampleToGroup> getSampleGroupMapping(const Box &root, IReport *out, uint32_t trackId)
{
  std::vector<SampleToGroup> res;

  // Check if mvex box is present
  bool fragmented = false;
  auto mvexBoxes = findBoxes(root, FOURCC("mvex"));
  if(!mvexBoxes.empty()) {
    fragmented = true;
  }

  // Check if moof box is present
  auto moofBoxes = findBoxes(root, FOURCC("moof"));
  if(!moofBoxes.empty() && !fragmented) {
    fragmented = true;
    out->warning("Fragmented file detected but no 'mvex' box is present to signal it");
  }

  auto processSamples = [&](Box const *trackBox) {
    // Get SampleGroupDescriptionBox
    auto sgpdBoxes = findBoxes(*trackBox, FOURCC("sgpd"));
    for(auto &sgpdBox : sgpdBoxes) {
      // Get grouping_type
      uint32_t grouping_type = 0;
      for(auto &sym : sgpdBox->syms) {
        if(!strcmp(sym.name, "grouping_type"))
          grouping_type = sym.value;
      }

      // Get SampleToGroupBox
      auto sbgpBoxes = findBoxes(*trackBox, FOURCC("sbgp"));
      for(auto &sbgpBox : sbgpBoxes) {
        // Get grouping_type
        uint32_t grouping_type_sbgp = 0;
        for(auto &sym : sbgpBox->syms) {
          if(!strcmp(sym.name, "grouping_type"))
            grouping_type_sbgp = sym.value;
        }

        // Check if grouping_type matches
        if(grouping_type != grouping_type_sbgp)
          continue;

        // Go through all entries
        uint32_t last_sample = 0;
        uint32_t sample_count = 0;
        uint32_t group_description_index = 0;
        for(auto &sym : sbgpBox->syms) {
          if(!strcmp(sym.name, "sample_count"))
            sample_count = sym.value;
          if(!strcmp(sym.name, "group_description_index")) {
            group_description_index = sym.value;

            // Create SampleGroupEntry
            SampleGroup sampleGroup{ grouping_type, group_description_index };

            // Add to result for each sample
            for(auto i = 0u; i < sample_count; i++) {
              // Check if sample already exists
              if(res.size() <= last_sample + i) {
                // Create new SampleToGroup
                SampleToGroup sampleToGroup{};
                if(group_description_index != 0)
                  sampleToGroup.push_back(sampleGroup);
                res.push_back(sampleToGroup);
              } else {
                // Add to existing SampleToGroup
                if(group_description_index != 0)
                  res[last_sample + i].push_back(sampleGroup);
              }
            }
            last_sample += sample_count;
          }
        }
      }
    }
  };

  if(fragmented) {
    // Get all movie fragments
    auto moofBoxes = findBoxes(root, FOURCC("moof"));
    for(auto &moofBox : moofBoxes) {
      // Get all track fragments
      auto trafBoxes = findBoxes(*moofBox, FOURCC("traf"));
      for(auto &trafBox : trafBoxes) {
        // Get track fragment header
        auto tfhdBoxes = findBoxes(*trafBox, FOURCC("tfhd"));
        if(tfhdBoxes.size() != 1) {
          out->error("%llu 'tfhd' boxes found, when 1 is expected", tfhdBoxes.size());
          return {};
        }

        uint32_t thisTrackId = 0;
        for(auto &sym : tfhdBoxes[0]->syms) {
          if(!strcmp(sym.name, "track_ID"))
            thisTrackId = sym.value;
        }

        if(thisTrackId != trackId) {
          continue;
        }

        processSamples(trafBox);
      }
    }
  } else {
    auto trakBoxes = findBoxes(root, FOURCC("trak"));
    for(auto &trakBox : trakBoxes) {
      auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
      if(tkhdBoxes.size() != 1) {
        // SKIP TRACK
        continue;
      }

      uint32_t thisTrackId = 0;
      for(auto &sym : tkhdBoxes[0]->syms)
        if(!strcmp(sym.name, "track_ID"))
          thisTrackId = sym.value;

      if(thisTrackId != trackId) {
        continue;
      }

      processSamples(trakBox);
    }
  }
  return res;
}

bool sampleHasGroup(const std::vector<SampleToGroup> &sampleToGroup, uint32_t sampleIndex, uint32_t grouping_type)
{
  if(sampleIndex >= sampleToGroup.size())
    return false;

  for(auto &sampleGroup : sampleToGroup[sampleIndex]) {
    if(sampleGroup.grouping_type == grouping_type)
      return true;
  }
  return false;
}

const SpecDesc specAv1ISOBMFF = {
  "av1isobmff",
  "AV1 Codec ISO Media File Format Binding v1.2.0, 12 December 2019\n"
  "https://github.com/AOMediaCodec/av1-isobmff/commit/"
  "ee2f1f0d2c342478206767fb4b79a39870c0827e\n"
  "https://aomediacodec.github.io/av1-isobmff/v1.2.0.html",
  { "isobmff" },
  {
    { "Section 2.1\n"
      "It SHALL have the 'av01' brand among the compatible brands array of the FileTypeBox",
      [](Box const &root, IReport *out) {
        auto ftyps = findBoxes(root, FOURCC("ftyp"));

        bool foundAv01 = false;
        for(auto &ftyp : ftyps) {
          for(auto &sym : ftyp->syms) {
            if(std::string(sym.name) == "compatible_brand" && sym.value == FOURCC("av01")) {
              foundAv01 = true;
              break;
            }
          }
        }

        if(!foundAv01) {
          out->error("No 'av01' found in compatibleBrands");
          return;
        }

        out->covered();
      } },
    { "Section 2.1\n"
      "It SHOULD indicate a structural ISOBMFF brand among the compatible brands array of\n"
      "the FileTypeBox",
      [](Box const &root, IReport *out) {
        auto ftyps = findBoxes(root, FOURCC("ftyp"));

        bool foundStructural = false;
        for(auto &ftyp : ftyps) {
          for(auto &sym : ftyp->syms) {
            if(std::string(sym.name) == "compatible_brand") {
              switch(sym.value) {
              case FOURCC("isom"):
              case FOURCC("iso2"):
              case FOURCC("iso4"):
              case FOURCC("iso6"):
                foundStructural = true;
                break;
              default:
                break;
              }
            }
            if(foundStructural) {
              break;
            }
          }
        }

        if(!foundStructural) {
          return;
        }

        out->covered();
      } },
    { "Section 2.1\n"
      "It SHALL contain at least one track using an AV1SampleEntry",
      [](Box const &root, IReport *out) {
        bool av01Found = false;
        auto stsdBoxes = findBoxes(root, FOURCC("stsd"));

        for(auto &stsdBox : stsdBoxes) {
          auto av01Boxes = findBoxes(*stsdBox, FOURCC("av01"));
          if(!av01Boxes.empty()) {
            av01Found = true;
            break;
          }
        }

        if(!av01Found) {
          out->error("No Av1SampleEntry found");
          return;
        }

        out->covered();
      } },
    { "Section 2.2.4\n"
      "The width and height fields of the VisualSampleEntry SHALL equal the values of\n"
      "max_frame_width_minus_1 + 1 and max_frame_height_minus_1 + 1 of the Sequence Header\n"
      "OBU applying to the samples associated with this sample entry.",
      [](Box const &root, IReport *out) {
        auto obuDetails = getOBUDetails(root, out);
        if(!obuDetails.valid) {
          return;
        }

        auto av01Details = getAv01Details(root);
        if(!av01Details.valid) {
          return;
        }

        if(av01Details.width != obuDetails.width || av01Details.height != obuDetails.height) {
          out->error(
            "Width and height of the VisualSampleEntry (%lldx%lld) don't match with the Sequence Header OBU "
            "(%lldx%lld)",
            av01Details.width, av01Details.height, obuDetails.width, obuDetails.height);
          return;
        }

        out->covered();
      } },
    { "Section 2.2.4\n"
      "The width and height in the TrackHeaderBox SHOULD equal, respectively, the maximum\n"
      "RenderWidth, called MaxRenderWidth, and the maximum RenderHeight, called\n"
      "MaxRenderHeight, of all the frames associated with this sample entry",
      [](Box const &root, IReport *out) {
        auto obuDetails = getOBUDetails(root, out);
        if(!obuDetails.valid) {
          return;
        }

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          if(tkhdBoxes.size() != 1) {
            out->error("%llu 'tkhd' boxes found, when 1 is expected", tkhdBoxes.size());
            return;
          }

          ResolutionDetails tkhdResolution{ false };
          for(auto &sym : tkhdBoxes[0]->syms) {
            if(std::string(sym.name) == "width") {
              tkhdResolution.width = sym.value >> 16;
            }
            if(std::string(sym.name) == "height") {
              tkhdResolution.height = sym.value >> 16;
            }
          }
          tkhdResolution.valid = tkhdResolution.width && tkhdResolution.height;

          if(obuDetails.width != tkhdResolution.width || obuDetails.height != tkhdResolution.height) {
            out->warning(
              "MaxRenderWidth/MaxRenderHeight (%lldx%lld) differ to tkhd box (%lldx%lld)", obuDetails.width,
              obuDetails.height, tkhdResolution.width, tkhdResolution.height);
            return;
          }
        }

        out->covered();
      } },
    { "Section 2.2.4\n"
      "Additionally, if MaxRenderWidth and MaxRenderHeight values do not equal respectively\n"
      "the max_frame_width_minus_1 + 1 and max_frame_height_minus_1 + 1 values of the\n"
      "Sequence Header OBU, a PixelAspectRatioBox box SHALL be present in the sample entry",
      [](Box const &root, IReport *out) {
        auto obuDetails = getOBUDetails(root, out);
        if(!obuDetails.valid) {
          return;
        }

        auto av01Boxes = findBoxes(root, FOURCC("av01"));
        for(auto av01Box : av01Boxes) {
          int vSpace = 0, hSpace = 0;
          for(auto &sym : av01Box->syms) {
            if(std::string(sym.name) == "horizresolution") {
              vSpace = sym.value >> 16;
            }
            if(std::string(sym.name) == "vertresolution") {
              hSpace = sym.value >> 16;
            }
          }

          bool expectPixelAspectRatio = vSpace != hSpace;

          if(!expectPixelAspectRatio) {
            continue;
          }

          out->covered();

          auto paspBoxes = findBoxes(*av01Box, FOURCC("pasp"));
          if(paspBoxes.size() != 1) {
            out->error("%llu 'pasp' boxes found, when 1 is expected", paspBoxes.size());
            return;
          }

          auto hSpacing = 0;
          auto vSpacing = 0;

          for(auto &sym : paspBoxes[0]->syms) {
            if(std::string(sym.name) == "hSpacing") {
              hSpacing = sym.value;
            }
            if(std::string(sym.name) == "vSpacing") {
              vSpacing = sym.value;
            }
          }

          double paspRatio = (double)hSpacing / vSpacing;
          double frameRatio = (double)(obuDetails.width * vSpace) / (obuDetails.height * hSpace);

          bool validPASP = (paspRatio - frameRatio < 0.001);

          if(!validPASP) {
            out->error("Invalid pasp: found %ux%u instead of %ux%u", hSpacing, vSpacing, hSpace, vSpace);
            return;
          }
        }
      } },
    { "Section 2.2.4\n"
      "The config field SHALL contain an AV1CodecConfigurationBox that applies to the samples\n"
      "associated with this sample entry.",
      [](Box const &root, IReport *out) {
        auto obuDetails = getOBUDetails(root, out);
        if(!obuDetails.valid) {
          return;
        }

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Boxes = findBoxes(*trakBox, FOURCC("av01"));
          for(auto &av01Box : av01Boxes) {
            auto av1CBoxes = findBoxes(*av01Box, FOURCC("av1C"));
            if(av1CBoxes.size() != 1) {
              out->error("%llu AV1CodecConfiguration box(es) found, expected 1", av1CBoxes.size());
              return;
            }
            if(!av1CBoxes.empty())
              out->covered();
          }
        }
      } },
    { "Section 2.3.4\n"
      "The AV1CodecConfigurationRecord marker field SHALL be set to 1",
      [](Box const &root, IReport *out) {
        auto obuDetails = getOBUDetails(root, out);
        if(!obuDetails.valid) {
          return;
        }

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av1CBoxes = findBoxes(*trakBox, FOURCC("av1C"));
          for(auto &av1CBox : av1CBoxes) {
            for(auto &sym : av1CBox->syms) {
              if(std::string(sym.name) == "marker" && sym.value != 1) {
                out->error("Marker SHALL be set to 1, found %lld", sym.value);
              }
              out->covered();
            }
          }
        }
      } },
    { "Section 2.3.4\n"
      "The AV1CodecConfigurationRecord version field SHALL be set to 1",
      [](Box const &root, IReport *out) {
        auto obuDetails = getOBUDetails(root, out);
        if(!obuDetails.valid) {
          return;
        }

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av1CBoxes = findBoxes(*trakBox, FOURCC("av1C"));
          for(auto &av1CBox : av1CBoxes) {
            for(auto &sym : av1CBox->syms) {
              if(std::string(sym.name) == "version" && sym.value != 1) {
                out->error("Version SHALL be set to 1, found %lld", sym.value);
              }
              out->covered();
            }
          }
        }
      } },
    { "Section 2.3.4\n"
      "The seq_profile field indicates the AV1 profile and SHALL be equal to\n"
      "the seq_profile value from the Sequence Header OBU.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            if(parseAv1Configs(
                                 root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                              if(av1cRef.seq_profile != bsState.av1c.seq_profile)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox seq_profile field value (%lld) SHALL be\n"
                                  "equal to the seq_profile value from the first Sequence Header OBU in the mdat "
                                  "(%lld)",
                                  trackId, av1cRef.seq_profile, bsState.av1c.seq_profile);

                              if(av1cRef.seq_profile != av1cState.av1c.seq_profile)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox seq_profile field value (%lld) SHALL be\n"
                                  "equal to the seq_profile value from the first Sequence Header OBU in configOBUS "
                                  "(%lld)",
                                  trackId, av1cRef.seq_profile, av1cState.av1c.seq_profile);
                            }
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The seq_level_idx_0 field indicates the value of seq_level_idx[0] found in the\n"
      "Sequence Header OBU and SHALL be equal to the value of seq_level_idx[0] in the\n"
      "Sequence Header OBU.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            if(parseAv1Configs(
                                 root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                              if(av1cRef.seq_level_idx_0 != bsState.av1c.seq_level_idx_0)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox seq_level_idx_0 field value (%lld) SHALL "
                                  "be\n"
                                  "equal to the seq_level_idx_0 value from the first Sequence Header OBU in the mdat "
                                  "(%lld)",
                                  trackId, av1cRef.seq_level_idx_0, bsState.av1c.seq_level_idx_0);

                              if(av1cRef.seq_level_idx_0 != av1cState.av1c.seq_level_idx_0)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox seq_level_idx_0 field value (%lld) SHALL "
                                  "be\n"
                                  "equal to the seq_level_idx_0 value from the first Sequence Header OBU in configOBUS "
                                  "(%lld)",
                                  trackId, av1cRef.seq_level_idx_0, av1cState.av1c.seq_level_idx_0);
                            }
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The seq_tier_0 field indicates the value of seq_tier[0] found in the\n"
      "Sequence Header OBU and SHALL be equal to the value of seq_tier[0] in the\n"
      "Sequence Header OBU.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            if(parseAv1Configs(
                                 root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                              if(av1cRef.seq_tier_0 != bsState.av1c.seq_tier_0)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox seq_tier_0 field value (%lld) SHALL be\n"
                                  "equal to the seq_tier_0 value from the first Sequence Header OBU in the mdat (%lld)",
                                  trackId, av1cRef.seq_tier_0, bsState.av1c.seq_tier_0);

                              if(av1cRef.seq_tier_0 != av1cState.av1c.seq_tier_0)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox seq_tier_0 field value (%lld) SHALL be\n"
                                  "equal to the seq_tier_0 value from the first Sequence Header OBU in configOBUS "
                                  "(%lld)",
                                  trackId, av1cRef.seq_tier_0, av1cState.av1c.seq_tier_0);
                            }
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The high_bitdepth field indicates the value of the high_bitdepth flag from the\n"
      "Sequence Header OBU.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            if(parseAv1Configs(
                                 root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                              if(av1cRef.high_bitdepth != bsState.av1c.high_bitdepth)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox high_bitdepth field value (%lld) SHALL "
                                  "be\n"
                                  "equal to the high_bitdepth value from the first Sequence Header OBU in the mdat "
                                  "(%lld)",
                                  trackId, av1cRef.high_bitdepth, bsState.av1c.high_bitdepth);

                              if(av1cRef.high_bitdepth != av1cState.av1c.high_bitdepth)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox high_bitdepth field value (%lld) SHALL "
                                  "be\n"
                                  "equal to the high_bitdepth value from the first Sequence Header OBU in configOBUS "
                                  "(%lld)",
                                  trackId, av1cRef.high_bitdepth, av1cState.av1c.high_bitdepth);
                            }
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The twelve_bit field indicates the value of the twelve_bit flag from the\n"
      "Sequence Header OBU. When twelve_bit is not present in the Sequence Header\n"
      "OBU the AV1CodecConfigurationRecord twelve_bit value SHALL be 0.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            if(parseAv1Configs(
                                 root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                              if(av1cRef.twelve_bit != bsState.av1c.twelve_bit)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox twelve_bit field value (%lld) SHALL be\n"
                                  "equal to the twelve_bit value from the first Sequence Header OBU in the mdat (%lld)",
                                  trackId, av1cRef.twelve_bit, bsState.av1c.twelve_bit);

                              if(av1cRef.twelve_bit != av1cState.av1c.twelve_bit)
                                out->error(
                                  "[TrackId=%u] The AV1CodecConfigurationBox twelve_bit field value (%lld) SHALL be\n"
                                  "equal to the twelve_bit value from the first Sequence Header OBU in configOBUS "
                                  "(%lld)",
                                  trackId, av1cRef.twelve_bit, av1cState.av1c.twelve_bit);
                            }
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The configOBUs field SHALL contain at most one Sequence Header OBU and if present, it SHALL be the first OBU.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            parseAv1Configs(
                              root, out, trackId, minfChild, bsState, av1cState, av1cRef, true, false, false);
                          }
              }
      } },
    { "Section 2.3.4\n"
      "ConfigOBUs: the flag obu_has_size_field SHALL be set to 1.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            parseAv1Configs(
                              root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, true, false);
                          }
              }
      } },
    { "Section 2.3.4\n"
      "When a Sequence Header OBU is contained within the configOBUs of the\n"
      "AV1CodecConfigurationRecord, the values present in the Sequence Header OBU\n"
      "contained within configOBUs SHALL match the values of the\n"
      "AV1CodecConfigurationRecord.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            if(parseAv1Configs(
                                 root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                              if(memcmp(&av1cState.av1c, &av1cRef, sizeof(AV1CodecConfigurationRecord)))
                                out->error(
                                  "[TrackId=%u] The values of the AV1CodecConfigurationBox shall match\n"
                                  "the Sequence Header OBU in the AV1 Image Item Data:\n"
                                  "\tAV1CodecConfigurationBox:\n%s\n"
                                  "\tSequence Header OBU in the AV1 Image Item Data:\n%s\n",
                                  trackId, av1cRef.toString().c_str(), av1cState.av1c.toString().c_str());
                            }
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The timing_info_present_flag in the Sequence Header OBU (in the configOBUs field or in the associated samples) "
      "SHOULD be set to 0.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            Av1State bsState, av1cState;
                            AV1CodecConfigurationRecord av1cRef{};
                            parseAv1Configs(
                              root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, true);
                          }
              }
      } },
    { "Section 2.3.4\n"
      "The sample entry SHOULD contain a colr box with a colour_type set to nclx.\n"
      "If present, the values of colour_primaries, transfer_characteristics, and\n"
      "matrix_coefficients SHALL match the values given in the Sequence Header OBU (in\n"
      "the configOBUs field or in the associated samples) if the\n"
      "color_description_present_flag is set to 1. Similarly, the full_range_flag in\n"
      "the colr box shall match the color_range flag in the Sequence Header OBU. When\n"
      "configOBUs does not contain a Sequence Header OBU, this box with colour_type set\n"
      "to nclx SHALL be present.",
      [](Box const & /*root*/, IReport * /*out*/) {} },
    { "Section 2.3.4\n"
      "The CleanApertureBox clap SHOULD not be present.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t trackId = 0;
                bool foundAv1C = false;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd")) {
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        trackId = sym.value;
                  } else if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl"))
                            for(auto &stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto &stsdChild : stblChild.children)
                                  if(stsdChild.fourcc == FOURCC("av01")) {
                                    for(auto &sampleEntryChild : stsdChild.children)
                                      if(sampleEntryChild.fourcc == FOURCC("av1C"))
                                        foundAv1C = true;

                                    for(auto &sampleEntryChild : stsdChild.children)
                                      if(sampleEntryChild.fourcc == FOURCC("clap"))
                                        if(foundAv1C)
                                          out->warning(
                                            "[TrackId=%u] The CleanApertureBox clap SHOULD not be present.", trackId);
                                  }
              }
      } },
    { "Section 2.3.4\n"
      "For sample entries corresponding to HDR content, the\n"
      "MasteringDisplayColourVolumeBox mdcv and ContentLightLevelBox clli SHOULD be\n"
      "present, and their values SHALL match the values of contained in the Metadata\n"
      "OBUs of type METADATA_TYPE_HDR_CLL and METADATA_TYPE_HDR_MDCV, if present (in\n"
      "the configOBUs or in the samples).",
      [](Box const &root, IReport *out) {
        struct MDCV {
          bool valid = false;
          uint16_t primary_chromaticity_x[3]; // display_primaries_x
          uint16_t primary_chromaticity_y[3]; // display_primaries_y
          uint16_t white_point_chromaticity_x; // white_point_x
          uint16_t white_point_chromaticity_y; // white_point_y
          uint32_t luminance_max; // max_display_mastering_luminance
          uint32_t luminance_min; // min_display_mastering_luminance
        };
        struct CLL {
          bool valid = false;
          uint16_t max_cll; // max_content_light_level
          uint16_t max_fall; // max_pic_average_light_level
        };

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          // See if MDCV and CLL are present in this sample
          MDCV in_sample_mdcv{};
          CLL in_sample_cll{};
          for(auto sampleIndex = 0u; sampleIndex < samples.size(); sampleIndex++) {
            BoxReader br;
            br.br = BitReader{ samples[sampleIndex].position, (int)samples[sampleIndex].size };

            Av1State state;
            while(!br.empty()) {
              auto firstSymIdx = br.myBox.syms.size();
              auto type = parseAv1Obus(&br, state, false);
              auto lastSymIdx = br.myBox.syms.size() - 1;

              if(type == 0)
                break;

              if(type == OBU_METADATA) {
                if(state.metadata_type == METADATA_TYPE_HDR_CLL) {
                  for(auto j = firstSymIdx; j <= lastSymIdx; j++) {
                    auto &sym = br.myBox.syms[j];
                    if(!strcmp(sym.name, "max_cll"))
                      in_sample_cll.max_cll = sym.value;
                    if(!strcmp(sym.name, "max_fall")) {
                      in_sample_cll.max_fall = sym.value;
                      in_sample_cll.valid = true;
                    }
                  }
                }
                if(state.metadata_type == METADATA_TYPE_HDR_MDCV) {
                  uint8_t xc = 0, yc = 0;
                  for(auto j = firstSymIdx; j <= lastSymIdx; j++) {
                    auto &sym = br.myBox.syms[j];
                    if(!strcmp(sym.name, "primary_chromaticity_x"))
                      in_sample_mdcv.primary_chromaticity_x[xc++] = sym.value;
                    if(!strcmp(sym.name, "primary_chromaticity_y"))
                      in_sample_mdcv.primary_chromaticity_y[yc++] = sym.value;
                    if(!strcmp(sym.name, "white_point_chromaticity_x"))
                      in_sample_mdcv.white_point_chromaticity_x = sym.value;
                    if(!strcmp(sym.name, "white_point_chromaticity_y"))
                      in_sample_mdcv.white_point_chromaticity_y = sym.value;
                    if(!strcmp(sym.name, "luminance_max"))
                      in_sample_mdcv.luminance_max = sym.value;
                    if(!strcmp(sym.name, "luminance_min")) {
                      in_sample_mdcv.luminance_min = sym.value;
                      in_sample_mdcv.valid = true;
                    }
                  }
                }
              }

              if(in_sample_cll.valid || in_sample_mdcv.valid)
                out->covered();

              if(in_sample_cll.valid && in_sample_mdcv.valid)
                break;
            }

            if(in_sample_cll.valid ^ in_sample_mdcv.valid) {
              out->error(
                "[TrackId=%u] Sample %u contains only one of METADATA_TYPE_HDR_CLL and METADATA_TYPE_HDR_MDCV", trackId,
                sampleIndex);
              return;
            }
          }

          // See if MDCV and CLL are present in the sample entry
          auto av01Boxes = findBoxes(*trakBox, FOURCC("av01"));
          for(auto &av01Box : av01Boxes) {
            // Parse av1C box in this samply entry
            auto av1cBoxes = findBoxes(*av01Box, FOURCC("av1C"));
            if(av1cBoxes.empty()) {
              out->error("[TrackId=%u] Sample Entry does not contain an av1C box", trackId);
              return;
            }

            // Parse obu array in this av1C box
            MDCV in_av1C_mdcv{};
            CLL in_av1C_cll{};

            for(auto &av1c : av1cBoxes) {
              bool isMetadata = false;
              uint8_t metadata_type = 0;

              for(auto &sym : av1c->syms) {
                if(!strcmp(sym.name, "metadata"))
                  isMetadata = true;
                if(!strcmp(sym.name, "/metadata")) {
                  isMetadata = false;
                  metadata_type = 0;
                }

                if(!isMetadata)
                  continue;

                if(!strcmp(sym.name, "leb128_byte")) {
                  metadata_type = sym.value;
                  continue;
                }

                if(metadata_type == METADATA_TYPE_HDR_CLL) {
                  if(!strcmp(sym.name, "max_cll"))
                    in_av1C_cll.max_cll = sym.value;
                  if(!strcmp(sym.name, "max_fall")) {
                    in_av1C_cll.max_fall = sym.value;
                    in_av1C_cll.valid = true;
                  }
                }
                if(metadata_type == METADATA_TYPE_HDR_MDCV) {
                  uint8_t xc = 0, yc = 0;
                  if(!strcmp(sym.name, "primary_chromaticity_x"))
                    in_av1C_mdcv.primary_chromaticity_x[xc++] = sym.value;
                  if(!strcmp(sym.name, "primary_chromaticity_y"))
                    in_av1C_mdcv.primary_chromaticity_y[yc++] = sym.value;
                  if(!strcmp(sym.name, "white_point_chromaticity_x"))
                    in_av1C_mdcv.white_point_chromaticity_x = sym.value;
                  if(!strcmp(sym.name, "white_point_chromaticity_y"))
                    in_av1C_mdcv.white_point_chromaticity_y = sym.value;
                  if(!strcmp(sym.name, "luminance_max"))
                    in_av1C_mdcv.luminance_max = sym.value;
                  if(!strcmp(sym.name, "luminance_min")) {
                    in_av1C_mdcv.luminance_min = sym.value;
                    in_av1C_mdcv.valid = true;
                  }
                }

                if(in_av1C_cll.valid && in_av1C_mdcv.valid)
                  break;
              }

              if(in_av1C_cll.valid ^ in_av1C_mdcv.valid) {
                out->error(
                  "[TrackId=%u] Sample Entry contains only one of METADATA_TYPE_HDR_CLL and "
                  "METADATA_TYPE_HDR_MDCV",
                  trackId);
                return;
              }
            }

            MDCV *mdcv = nullptr;
            CLL *cll = nullptr;

            // Select either the sample entry or the sample
            if(in_sample_cll.valid) {
              mdcv = &in_sample_mdcv;
              cll = &in_sample_cll;
            } else if(in_av1C_cll.valid) {
              mdcv = &in_av1C_mdcv;
              cll = &in_av1C_cll;
            } else {
              continue;
            }

            // Get mdcv and clli boxes
            auto mdcvBoxes = findBoxes(*av01Box, FOURCC("mdcv"));
            auto clliBoxes = findBoxes(*av01Box, FOURCC("clli"));

            // Check if they are present
            if(mdcvBoxes.empty() || clliBoxes.empty()) {
              out->error("[TrackId=%u] Sample Entry does not contain an mdcv or clli box", trackId);
              return;
            }

            // Check if they match
            for(auto &mdcvBox : mdcvBoxes) {
              uint8_t xc = 0, yc = 0;
              for(auto &sym : mdcvBox->syms) {
                if(!strcmp(sym.name, "display_primaries_x"))
                  if(sym.value != mdcv->primary_chromaticity_x[xc++])
                    out->error(
                      "[TrackId=%u] Sample Entry mdcv box display_primaries_x does not match the "
                      "METADATA_TYPE_HDR_MDCV",
                      trackId);
                if(!strcmp(sym.name, "display_primaries_y"))
                  if(sym.value != mdcv->primary_chromaticity_y[yc++])
                    out->error(
                      "[TrackId=%u] Sample Entry mdcv box display_primaries_y does not match the "
                      "METADATA_TYPE_HDR_MDCV",
                      trackId);
                if(!strcmp(sym.name, "white_point_x"))
                  if(sym.value != mdcv->white_point_chromaticity_x)
                    out->error(
                      "[TrackId=%u] Sample Entry mdcv box white_point_x does not match the "
                      "METADATA_TYPE_HDR_MDCV",
                      trackId);
                if(!strcmp(sym.name, "white_point_y"))
                  if(sym.value != mdcv->white_point_chromaticity_y)
                    out->error(
                      "[TrackId=%u] Sample Entry mdcv box white_point_y does not match the "
                      "METADATA_TYPE_HDR_MDCV",
                      trackId);
                if(!strcmp(sym.name, "max_display_mastering_luminance"))
                  if(sym.value != mdcv->luminance_max)
                    out->error(
                      "[TrackId=%u] Sample Entry mdcv box max_display_mastering_luminance does not match the "
                      "METADATA_TYPE_HDR_MDCV",
                      trackId);
                if(!strcmp(sym.name, "min_display_mastering_luminance"))
                  if(sym.value != mdcv->luminance_min)
                    out->error(
                      "[TrackId=%u] Sample Entry mdcv box min_display_mastering_luminance does not match the "
                      "METADATA_TYPE_HDR_MDCV",
                      trackId);
              }
            }

            for(auto &clliBox : clliBoxes) {
              for(auto &sym : clliBox->syms) {
                if(!strcmp(sym.name, "max_content_light_level"))
                  if(sym.value != cll->max_cll)
                    out->error(
                      "[TrackId=%u] Sample Entry clli box max_content_light_level does not match the "
                      "METADATA_TYPE_HDR_CLL",
                      trackId);
                if(!strcmp(sym.name, "max_pic_average_light_level"))
                  if(sym.value != cll->max_fall)
                    out->error(
                      "[TrackId=%u] Sample Entry clli box max_pic_average_light_level does not match the "
                      "METADATA_TYPE_HDR_CLL",
                      trackId);
              }
            }

            out->covered();
          }
        }
      } },
    { "Section 2.4\n"
      "The sample data SHALL be a sequence of OBUs forming a Temporal Unit",
      [](Box const &root, IReport *out) {
        auto av1Tracks = getAv1Tracks(root);

        for(auto &trackId : av1Tracks) {
          auto samples = getData(root, out, trackId);
          for(auto &sample : samples) {
            BoxReader br;
            br.br = sample.getSample();
            if(br.br.size < 2)
              return;

            Av1State stateUnused;
            auto foundTemporal = 0;
            while(!br.empty()) {
              auto obu_type = parseAv1Obus(&br, stateUnused, false);
              if(!obu_type) {
                out->error("Found an invalid obu in stream");
                return;
              }
              if(obu_type == OBU_TEMPORAL_DELIMITER) {
                foundTemporal++;
                if(foundTemporal > 1) {
                  out->error("Found more than 1 temporal delimiters in stream");
                  return;
                }
              }
              out->covered();
            }
          }
        }
      } },
    { "Section 2.4\n"
      "Each OBU SHALL follow the open_bitstream_unit Low Overhead Bitstream Format\n"
      "syntax as specified in [AV1]. Each OBU SHALL have the obu_has_size_field set\n"
      "to 1 except for the last OBU in the sample, for which obu_has_size_field MAY be\n"
      "set to 0, in which case it is assumed to fill the remainder of the sample",
      [](Box const &root, IReport *out) {
        auto av1Tracks = getAv1Tracks(root);

        for(auto &trackId : av1Tracks) {
          auto samples = getData(root, out, trackId);
          for(auto &sample : samples) {
            BoxReader br;
            br.br = sample.getSample();
            if(br.br.size < 2)
              return;

            Av1State stateUnused;
            auto withoutSize = 0;
            auto previousHasSize = true;
            while(!br.empty()) {
              auto obu_type = parseAv1Obus(&br, stateUnused, false);
              if(!obu_type) {
                out->error("Found an invalid obu in stream");
                return;
              }
              for(auto &it : br.myBox.syms) {
                if(std::string(it.name) == "obu_has_size_field") {
                  if(it.value == 0) {
                    withoutSize++;
                  }
                  previousHasSize = it.value;
                  break;
                }
              }
              out->covered();
            }
            if(withoutSize > 1 || (withoutSize == 1 && previousHasSize)) {
              out->error(
                "Found %d obu's without size and the last does %shave a size", withoutSize,
                (previousHasSize ? "" : "not "));
            }
          }
        }
      } },
    { "Section 2.4\n"
      "OBU trailing bits SHOULD be limited to byte alignment and SHOULD not be used for padding",
      [](Box const &root, IReport *out) {
        auto av1Tracks = getAv1Tracks(root);

        for(auto &trackId : av1Tracks) {
          auto samples = getData(root, out, trackId);
          size_t i = 0;
          for(auto &sample : samples) {

            if(sample.position[sample.size - 1] == 0x00) {
              out->warning("[TrackId=%u] Sample %zu contains more extra trailing bits", trackId, i);
            }

            out->covered();
            i++;
          }
        }
      } },
    { "Section 2.4\n"
      "OBUs of type OBU_TILE_LIST SHALL NOT be used.",
      [](Box const &root, IReport *out) {
        auto av1Tracks = getAv1Tracks(root);

        for(auto &trackId : av1Tracks) {
          auto samples = getData(root, out, trackId);
          for(auto &sample : samples) {
            BoxReader br;
            br.br = sample.getSample();

            if(br.br.size < 2)
              return;

            Av1State stateUnused;
            while(!br.empty()) {
              auto obu_type = parseAv1Obus(&br, stateUnused, false);
              if(!obu_type) {
                out->error("Found an invalid obu in stream");
                return;
              }
              if(obu_type == OBU_TILE_LIST) {
                out->error("Tile list obu found in stream");
                return;
              }
              out->covered();
            }
          }
        }
      } },
    { "Section 2.4\n"
      "OBUs of type OBU_TEMPORAL_DELIMITER, OBU_PADDING, or OBU_REDUNDANT_FRAME_HEADER SHOULD NOT be used.",
      [](Box const &root, IReport *out) {
        auto av1Tracks = getAv1Tracks(root);

        for(auto &trackId : av1Tracks) {
          auto samples = getData(root, out, trackId);
          for(auto &sample : samples) {
            BoxReader br;
            br.br = sample.getSample();

            if(br.br.size < 2)
              return;

            auto obuTemporalDelimiter = 0;
            auto obuPadding = 0;
            auto obuRedundantFrameHeader = 0;
            Av1State stateUnused;
            while(!br.empty()) {
              auto obu_type = parseAv1Obus(&br, stateUnused, false);
              if(!obu_type) {
                out->error("Found an invalid obu in stream");
                return;
              }
              if(obu_type == OBU_TEMPORAL_DELIMITER) {
                obuTemporalDelimiter++;
              }
              if(obu_type == OBU_PADDING) {
                obuPadding++;
              }
              if(obu_type == OBU_REDUNDANT_FRAME_HEADER) {
                obuRedundantFrameHeader++;
              }
              out->covered();
            }
            if(obuTemporalDelimiter) {
              out->warning("Found %d OBU_TEMPORAL_DELIMITER obu(s)", obuTemporalDelimiter);
            }
            if(obuPadding) {
              out->warning("Found %d OBU_PADDING obu(s)", obuPadding);
            }
            if(obuRedundantFrameHeader) {
              out->warning("Found %d OBU_REDUNDANT_FRAME_HEADER obu(s)", obuRedundantFrameHeader);
            }
          }
        }
      } },
    { "Section 2.4\n"
      "Intra-only frames SHOULD be signaled using the sample_depends_on flag set to 2.",
      [](Box const &root, IReport *out) {
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto &tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          for(auto i = 0u; i < samples.size(); i++) {
            uint8_t sampleDependsOn = samples[i].flags.sampleDependsOn;

            BoxReader br;
            br.br = BitReader{ samples[i].position, (int)samples[i].size };

            Av1State stateUnused;
            while(!br.empty()) {
              auto firstSymIdx = br.myBox.syms.size();
              auto type = parseAv1Obus(&br, stateUnused, false);
              auto lastSymIdx = br.myBox.syms.size() - 1;

              if(type == 0)
                break;

              if(type == OBU_FRAME_HEADER) {
                bool intraFrame = false;
                for(auto j = firstSymIdx; j <= lastSymIdx; j++) {
                  auto &it = br.myBox.syms[j];
                  if(!strcmp(it.name, "frame_type") && it.value == AV1_INTRA_ONLY_FRAME) {
                    intraFrame = true;
                    out->covered();
                  }
                }

                if(intraFrame && sampleDependsOn != 2) {
                  out->warning(
                    "[TrackId=%u] Sample %u contains an intra-only frame, but sample_depends_on is not set to 2",
                    trackId, i);
                  return;
                }
              }
            }
          }
        }
      } },
    { "Section 2.4\n"
      "Delayed Random Access Points SHOULD be signaled using sample groups and the\n"
      "AV1ForwardKeyFrameSampleGroupEntry.",
      [](Box const &root, IReport *out) {
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto &tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          // Get which samples belong to av1f
          auto sampleToGroupMap = getSampleGroupMapping(root, out, trackId);

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          for(auto sampleIndex = 0u; sampleIndex < samples.size(); sampleIndex++) {
            BoxReader br;
            br.br = BitReader{ samples[sampleIndex].position, (int)samples[sampleIndex].size };

            Av1State stateUnused;
            bool foundSequenceHeader = false, sawPartialDRAP = false;
            while(!br.empty()) {
              auto type = parseAv1Obus(&br, stateUnused, false);

              if(type == 0)
                break;

              if(type == OBU_TEMPORAL_DELIMITER) {
                foundSequenceHeader = false;
                sawPartialDRAP = false;
              }

              if(type == OBU_FRAME_HEADER && !sawPartialDRAP) {
                bool keyFrame = false, noShowFrame = false;
                for(auto &it : br.myBox.syms) {
                  if(!strcmp(it.name, "frame_type") && it.value == AV1_KEY_FRAME) {
                    keyFrame = true;
                  }
                  if(!strcmp(it.name, "show_frame") && it.value == 0) {
                    noShowFrame = true;
                  }
                }

                if(keyFrame && noShowFrame)
                  sawPartialDRAP = true;
              }

              if(type == OBU_SEQUENCE_HEADER)
                foundSequenceHeader = true;

              if(sawPartialDRAP && foundSequenceHeader)
                break;
            }

            if(sawPartialDRAP && foundSequenceHeader) {
              // Check if this sample is part of av1f
              if(!sampleHasGroup(sampleToGroupMap, sampleIndex, FOURCC("av1f"))) {
                out->warning(
                  "[TrackId=%u] Sample %u contains a delayed random access point, but is not part of an av1f sample "
                  "group",
                  trackId, sampleIndex);
                break;
              }

              out->covered();
            }
          }
        }
      } },
    { "Section 2.4\n"
      "Switch Frames SHOULD be signaled using sample groups and the AV1SwitchFrameSampleGroupEntry.",
      [](Box const &root, IReport *out) {
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          // Get which samples belong to av1s
          auto sampleToGroupMap = getSampleGroupMapping(root, out, trackId);

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          for(auto sampleIndex = 0u; sampleIndex < samples.size(); sampleIndex++) {
            BoxReader br;
            br.br = BitReader{ samples[sampleIndex].position, (int)samples[sampleIndex].size };

            Av1State stateUnused;
            bool foundSwitchFrame = false;
            while(!br.empty()) {
              auto type = parseAv1Obus(&br, stateUnused, false);

              if(type == 0)
                break;

              if(type == OBU_FRAME_HEADER) {
                for(auto &it : br.myBox.syms) {
                  if(!strcmp(it.name, "frame_type") && it.value == AV1_SWITCH_FRAME) {
                    foundSwitchFrame = true;
                    out->covered();
                    break;
                  }
                }
              }

              if(foundSwitchFrame)
                break;
            }

            if(foundSwitchFrame) {
              // Check if this sample is part of av1s
              if(!sampleHasGroup(sampleToGroupMap, sampleIndex, FOURCC("av1s"))) {
                out->warning(
                  "[TrackId=%u] Sample %u contains a Switch Frame, but is not part of an av1s sample "
                  "group",
                  trackId, sampleIndex);
                break;
              }
            }
          }
        }
      } },
    { "Section 2.4\n"
      "If a file contains multiple tracks that are alternative representations of the\n"
      "same content, in particular using Switch Frames, those tracks SHOULD be marked\n"
      "as belonging to the same alternate group and should use a track selection box\n"
      "with an appropriate attribute (e.g. bitr).",
      [](Box const & /*root*/, IReport * /*out*/) {
        // TODO: if the elementary stream contains Switch Frames and the corresponding ISOBMFF track is not part of an
        // alternate group, emit a warning? alternate group=tkhd
        //! It's not possible to implement this, as we don't know which tracks are alternative represnetations to each
        //! other.
        /*
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for (auto tkhd : tkhdBoxes) {
            for (auto &sym : tkhd->syms)
              if (sym.name == std::string("alternate_group") && sym.value == 0) {
                out->warning("Track %s is not part of an alternate group", av01Details.trackName.c_str());
              }
          }
        }
        */
      } },
    { "Section 2.4\n"
      "Metadata OBUs may be carried in sample data. In this case, the\n"
      "AV1MetadataSampleGroupEntry SHOULD be used.",
      [](Box const &root, IReport *out) {
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          // Get which samples belong to av1M
          auto sampleToGroupMap = getSampleGroupMapping(root, out, trackId);

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          for(auto sampleIndex = 0u; sampleIndex < samples.size(); sampleIndex++) {
            BoxReader br;
            br.br = BitReader{ samples[sampleIndex].position, (int)samples[sampleIndex].size };

            Av1State stateUnused;
            bool foundMetadata = false;
            while(!br.empty()) {
              auto type = parseAv1Obus(&br, stateUnused, false);

              if(type == 0)
                break;

              if(type == OBU_METADATA) {
                foundMetadata = true;
                break;
              }
            }

            if(foundMetadata) {
              // Check if this sample is part of av1M
              if(!sampleHasGroup(sampleToGroupMap, sampleIndex, FOURCC("av1M"))) {
                out->warning(
                  "[TrackId=%u] Sample %u contains a Metadata OBU, but is not part of an av1M sample "
                  "group",
                  trackId, sampleIndex);
                break;
              }

              out->covered();
            }
          }
        }
      } },
    { "Section 2.4\n"
      "If the metadata OBUs are static for the entire set of samples associated with a\n"
      "given sample description entry, they SHOULD also be in the OBU array in the\n"
      "sample description entry.",
      [](Box const &root, IReport *out) {
        // Determine if frag or non-frag
        // Check if mvex box is present
        bool fragmented = false;
        auto mvexBoxes = findBoxes(root, FOURCC("mvex"));
        if(!mvexBoxes.empty()) {
          fragmented = true;
        }

        // Check if moof box is present
        auto moofBoxes = findBoxes(root, FOURCC("moof"));
        if(!moofBoxes.empty() && !fragmented) {
          fragmented = true;
          out->warning("Fragmented file detected but no 'mvex' box is present to signal it");
        }

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto &tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          // Get sample_description_index from stsc box (non-frag) or default_sample_description_index from trex box
          std::vector<uint32_t> sampleDescriptionIndices;
          uint32_t defaultSampleDescriptionIndex = 1;
          if(fragmented) {
            // Get default_sample_description_index from trex box
            auto trexBoxes = findBoxes(root, FOURCC("trex"));
            for(auto &trex : trexBoxes) {
              // Get track_ID
              uint32_t thisTrackId = 0;
              for(auto &sym : trex->syms)
                if(!strcmp(sym.name, "track_ID"))
                  thisTrackId = sym.value;

              if(thisTrackId != trackId)
                continue;

              for(auto &sym : trex->syms) {
                if(!strcmp(sym.name, "default_sample_description_index")) {
                  defaultSampleDescriptionIndex = sym.value;
                }
              }
            }
          } else {
            // Get sample_description_index from stsc box
            auto stscBoxes = findBoxes(*trakBox, FOURCC("stsc"));
            for(auto &stsc : stscBoxes) {
              uint32_t sampleCount = 0;
              for(auto &sym : stsc->syms) {
                if(!strcmp(sym.name, "samples_per_chunk"))
                  sampleCount = sym.value;
                if(!strcmp(sym.name, "sample_description_index")) {
                  for(auto i = 0u; i < sampleCount; i++) {
                    sampleDescriptionIndices.push_back(sym.value);
                  }
                }
              }
            }
          }

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          struct OBU {
            int64_t type = 0;
            std::vector<uint32_t> data;
          };
          std::map<uint32_t, std::vector<OBU>> sampleToObuMap;

          for(auto i = 0u; i < samples.size(); i++) {
            if(!fragmented && i >= sampleDescriptionIndices.size())
              continue;

            auto sample_description_index = fragmented ? defaultSampleDescriptionIndex : sampleDescriptionIndices[i];
            BoxReader br;
            br.br = BitReader{ samples[i].position, (int)samples[i].size };

            Av1State stateUnused;
            while(!br.empty()) {
              OBU obu;
              auto firstSymIdx = br.myBox.syms.size();
              obu.type = parseAv1Obus(&br, stateUnused, false);
              auto lastSymIdx = br.myBox.syms.size() - 1;

              if(obu.type == 0)
                break;

              // Look at metadata OBUs and store their data
              if(obu.type == OBU_METADATA) {
                for(auto j = firstSymIdx; j <= lastSymIdx; j++) {
                  if(!strcmp(br.myBox.syms[j].name, "obu"))
                    continue;
                  obu.data.push_back(br.myBox.syms[j].value);
                }

                sampleToObuMap[sample_description_index].push_back(obu);

                out->covered();
              }
            }
          }

          // Check if metadata OBUs are static for the entire set of samples
          std::vector<uint32_t> staticSampleDescriptionIndices;
          for(auto &it : sampleToObuMap) {
            auto &obus = it.second;
            bool staticObus = false;
            if(obus.size() > 1) {
              staticObus = true;
              auto &firstObu = obus[0];
              for(auto &obu : obus) {
                if(obu.data != firstObu.data) {
                  staticObus = false;
                  break;
                }
              }
            }
            if(staticObus)
              staticSampleDescriptionIndices.push_back(it.first);
          }

          // For static metadata OBUs, check if they are in the sample description entry
          if(!staticSampleDescriptionIndices.empty()) {
            auto stsdBoxes = findBoxes(*trakBox, FOURCC("stsd"));
            for(auto &stsd : stsdBoxes) {
              uint32_t entryCount = 0;
              for(auto &sym : stsd->syms) {
                if(!strcmp(sym.name, "entry_count"))
                  entryCount = sym.value;
              }

              for(auto staticSampleEntry : staticSampleDescriptionIndices) {
                if(staticSampleEntry > entryCount) {
                  out->error(
                    "[TrackId=%u] Sample description entry %u is not present in the sample description entry", trackId,
                    staticSampleEntry);
                  return;
                }

                // Get sample description entry
                auto sampleEntry = stsd->children[staticSampleEntry - 1];

                // Parse av1C box in this samply entry
                auto av1cBox = findBoxes(sampleEntry, FOURCC("av1C"));
                if(av1cBox.empty()) {
                  out->error(
                    "[TrackId=%u] Sample description entry %u does not contain an av1C box", trackId,
                    staticSampleEntry);
                  return;
                }

                // Parse obu array in this av1C box
                std::vector<OBU> metadataObus;
                for(auto &av1c : av1cBox) {
                  std::vector<uint32_t> metadataObu;
                  bool foundMetadataObu = false;

                  for(auto &sym : av1c->syms) {
                    if(!strcmp(sym.name, "obu")) {
                      if(foundMetadataObu)
                        metadataObus.push_back({ OBU_METADATA, metadataObu });

                      foundMetadataObu = false;
                      metadataObu.clear();
                      continue;
                    }

                    if(!strcmp(sym.name, "obu_type")) {
                      if(sym.value == OBU_METADATA)
                        foundMetadataObu = true;
                    }

                    // Put the value in the metadata OBU
                    metadataObu.push_back(sym.value);
                  }

                  // Add the last metadata OBU
                  if(foundMetadataObu)
                    metadataObus.push_back({ OBU_METADATA, metadataObu });
                }

                // Check if the metadata OBUs are the same as the ones in the sample data
                auto &obus = sampleToObuMap[staticSampleEntry];
                bool foundSameOBU = false;
                for(auto &metadataObu : metadataObus) {
                  if(obus[0].data == metadataObu.data) {
                    foundSameOBU = true;
                    break;
                  }
                }
                if(!foundSameOBU) {
                  out->warning(
                    "[TrackId=%u] Metadata OBUs for sample description entry %u are static for the entire set of "
                    "samples "
                    "but are not present in the OBU array in the sample description entry",
                    trackId, staticSampleEntry);
                }
              }
            }
          }
        }
      } },
    { "Section 2.4\n"
      "If an AV1 Sample is signaled as a sync sample (in the SyncSampleBox or by\n"
      "setting sample_is_non_sync_sample to 0), it SHALL be a Random Access Point\n"
      "as defined in [AV1], i.e. satisfy the following constraints:\n"
      "- Its first frame is a Key Frame that has show_frame flag set to 1,\n"
      "- It contains a Sequence Header OBU before the first Frame Header OBU.",
      [](Box const &root, IReport *out) {
        // Check if mvex box is present
        bool fragmented = false;
        auto mvexBoxes = findBoxes(root, FOURCC("mvex"));
        if(!mvexBoxes.empty()) {
          fragmented = true;
        }

        // Check if moof box is present
        auto moofBoxes = findBoxes(root, FOURCC("moof"));
        if(!moofBoxes.empty() && !fragmented) {
          fragmented = true;
          out->warning("Fragmented file detected but no 'mvex' box is present to signal it");
        }

        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto &tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          std::vector<uint32_t> syncSamples;
          bool defaultSampleIsSyncSample = false;

          if(fragmented) {
            // Get sample_is_non_sync_sample from trex box (frag)
            auto trexBoxes = findBoxes(root, FOURCC("trex"));
            for(auto &trex : trexBoxes) {
              // Get track_ID
              uint32_t thisTrackId = 0;
              for(auto &sym : trex->syms)
                if(!strcmp(sym.name, "track_ID"))
                  thisTrackId = sym.value;

              if(thisTrackId != trackId)
                continue;

              for(auto &sym : trex->syms) {
                if(!strcmp(sym.name, "sample_is_non_sync_sample")) {
                  defaultSampleIsSyncSample = sym.value == 0;
                }
              }
            }
          } else {
            // Get stss and look which samples are sync (non-frag)
            auto stssBoxes = findBoxes(*trakBox, FOURCC("stss"));
            for(auto &stss : stssBoxes) {
              for(auto &sym : stss->syms) {
                if(!strcmp(sym.name, "sample_number")) {
                  syncSamples.push_back(sym.value);
                }
              }
            }
          }

          // If no sync sample, continue
          if(syncSamples.empty())
            continue;

          out->covered();

          // Get samples
          auto const samples = getData(root, out, trackId);
          if(samples.empty()) {
            out->warning("No sample found for trackId=%u", trackId);
            continue;
          }

          for(auto i = 0u; i < samples.size(); i++) {
            // if not sync sample, continue
            if(fragmented) {
              if(!defaultSampleIsSyncSample)
                continue;
            } else {
              if(std::find(syncSamples.begin(), syncSamples.end(), i + 1) == syncSamples.end())
                continue;
            }

            BoxReader br;
            br.br = BitReader{ samples[i].position, (int)samples[i].size };

            Av1State stateUnused;
            bool foundSequenceHeader = false, sawFrame = false;
            while(!br.empty()) {
              auto type = parseAv1Obus(&br, stateUnused, false);

              if(type == 0)
                break;

              if(type == OBU_SEQUENCE_HEADER) {
                foundSequenceHeader = true;
              }

              if(type == OBU_FRAME_HEADER && !foundSequenceHeader) {
                out->error(
                  "[TrackId=%u] Sample %u is a sync sample but its first frame does not contain a Sequence Header "
                  "OBU",
                  trackId, i);
                return;
              }

              if(type == OBU_FRAME_HEADER || type == OBU_FRAME) {
                bool keyFrame = false, showFrame = false;
                for(auto &it : br.myBox.syms) {
                  if(!strcmp(it.name, "frame_type") && it.value == AV1_KEY_FRAME) {
                    keyFrame = true;
                  }
                  if(!strcmp(it.name, "show_frame") && it.value) {
                    showFrame = true;
                  }
                }

                // if this is the first frame and it is not a key frame, error
                if(!sawFrame && !keyFrame) {
                  out->error(
                    "[TrackId=%u] Sample %u is a sync sample but its first frame is not a key frame", trackId, i);
                }

                // if this is the first frame and its show_frame flag is not set, error
                if(!sawFrame && !showFrame) {
                  out->error(
                    "[TrackId=%u] Sample %u is a sync sample but its first frame has its show_frame flag not set",
                    trackId, i);
                }

                sawFrame = true;
              }
            }
          }
        }
      } },
    { "Section 2.4\n"
      "In tracks using the AV1SampleEntry, the ctts box and composition offsets in\n"
      "movie fragments SHALL NOT be used.",
      [](Box const &root, IReport *out) {
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto &tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          auto cttsBoxes = findBoxes(*trakBox, FOURCC("ctts"));
          if(!cttsBoxes.empty()) {
            out->error("Track %d uses ctts box", trackId);
          }

          auto cslgBoxes = findBoxes(*trakBox, FOURCC("cslg"));
          if(!cslgBoxes.empty()) {
            out->error("Track %d uses cslg box", trackId);
          }

          // Check trun flags
          auto trafBoxes = findBoxes(root, FOURCC("traf"));
          for(auto &traf : trafBoxes) {
            // Get track_ID
            uint32_t thisTrackId = 0;
            auto tfhdBoxes = findBoxes(*traf, FOURCC("tfhd"));
            for(auto tfhd : tfhdBoxes) {
              for(auto &sym : tfhd->syms)
                if(!strcmp(sym.name, "track_ID"))
                  thisTrackId = sym.value;
            }

            if(thisTrackId != trackId)
              continue;

            auto trunBoxes = findBoxes(*traf, FOURCC("trun"));
            for(auto &trun : trunBoxes) {
              for(auto &sym : trun->syms) {
                if(!strcmp(sym.name, "flags") && (sym.value & 0x000800)) {
                  out->error("trun box with sample-composition-time-offsets-present flag");
                  break;
                }
              }
            }
          }

          out->covered();
        }
      } },
    { "Section 2.4\n"
      "In tracks using the AV1SampleEntry, the is_leading flag, if used,\n"
      "SHALL be set to 0 or 2.",
      [](Box const &root, IReport *out) {
        auto trakBoxes = findBoxes(root, FOURCC("trak"));
        for(auto &trakBox : trakBoxes) {
          auto av01Details = getAv01Details(*trakBox);
          if(!av01Details.valid) {
            continue;
          }

          // Get the track ID
          uint32_t trackId = 0;
          auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
          for(auto &tkhd : tkhdBoxes) {
            for(auto &sym : tkhd->syms)
              if(!strcmp(sym.name, "track_ID"))
                trackId = sym.value;
          }

          auto sdtpBoxes = findBoxes(*trakBox, FOURCC("sdtp"));
          for(auto &sdtp : sdtpBoxes) {
            for(auto &sym : sdtp->syms) {
              if(!strcmp(sym.name, "is_leading") && sym.value != 0 && sym.value != 2) {
                out->error("Track %d uses sdtp box with is_leading flag set to %d", trackId, sym.value);
                return;
              }
            }
          }

          out->covered();
        }
      } },
    { "Section 2.8.4\n"
      "metadata_specific_parameters is only defined when metadata_type is set to\n"
      "METADATA_TYPE_ITUT_T35 in which case its value SHALL be set to the first 24 bits\n"
      "of the metadata_itut_t35 structure. For other types of metadata,\n"
      "its [metadata_specific_parameters] value SHOULD be set to 0.",
      [](Box const &root, IReport *out) {
        for(auto &box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto &moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak")) {
                uint32_t thisTrackId = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd"))
                    for(auto &sym : trakChild.syms)
                      if(!strcmp(sym.name, "track_ID"))
                        thisTrackId = sym.value;

                bool foundAv1 = false;
                uint32_t av1M_metadata_specific_parameters = 0;

                for(auto &trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto &mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto &minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl")) {
                            for(auto &stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto &stsdChild : stblChild.children)
                                  if(stsdChild.fourcc == FOURCC("av01"))
                                    foundAv1 = true;

                            if(!foundAv1)
                              continue;

                            for(auto &stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("sbgp"))
                                for(auto &sym : stblChild.syms) {
                                  if(!strcmp(sym.name, "grouping_type_parameter")) {
                                    if(av1M_metadata_specific_parameters != 0) {
                                      out->error("Multiple av1M entries found: only considering the first one");
                                      break;
                                    }

                                    if((sym.value >> 24) == FOURCC("av1M"))
                                      av1M_metadata_specific_parameters = sym.value & 0x00FFFFFF;
                                  }
                                }

                            if(!av1M_metadata_specific_parameters)
                              continue;
                          }

                if(!av1M_metadata_specific_parameters)
                  continue;

                auto const samples = getData(root, out, thisTrackId);
                if(samples.empty()) {
                  out->warning("No sample found for trackId=%u", thisTrackId);
                  continue;
                }

                BoxReader br;
                br.br = BitReader{ samples[0].position, (int)samples[0].size };

                Av1State stateUnused;
                while(!br.empty()) {
                  parseAv1Obus(&br, stateUnused, false);
                }

                uint32_t metadata_type = 0;
                for(auto &sym : br.myBox.syms) {
                  if(!strcmp(sym.name, "itu_t_t35_country_code")) {
                    if(sym.value != 0xFF)
                      metadata_type = sym.value << 16;

                    out->covered();
                  } else if(!strcmp(sym.name, "itu_t_t35_country_code_extension_byte")) {
                    metadata_type = sym.value << 16;
                  } else if(!strcmp(sym.name, "itu_t_t35_terminal_provider_code")) {
                    metadata_type += sym.value;
                    if(metadata_type != av1M_metadata_specific_parameters) {
                      out->error(
                        "metadata_specific_parameters value (%u) SHALL be set to the first 24 bits "
                        "of the metadata_itut_t35 structure (%u)",
                        av1M_metadata_specific_parameters, metadata_type);
                      metadata_type = 0;
                    }
                  }
                }
              }
      } },
#if 0 // CMAF: not covered for now
    { "Section 3\n"
      "CMAF AV1 Tracks SHALL use an AV1SampleEntry",
      [](Box const & /*root*/, IReport * /*out*/) {
        // TODO
      } },
    { "Section 3\n"
      "CMAF AV1 Tracks MAY use multiple sample entries, and in that case the following\n"
      "values SHALL not change in the track:\n"
      "seq_profile\n"
      "still_picture\n"
      "the first value of seq_level_idx\n"
      "the first value of seq_tier\n"
      "color_config\n"
      "initial_presentation_delay_minus_one",
      [](Box const & /*root*/, IReport * /*out*/) {
        // TODO
      } },
#endif
  },
  isIsobmff,
};

static auto const registered = registerSpec(&specAv1ISOBMFF);
} // namespace
