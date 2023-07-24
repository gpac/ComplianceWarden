#include "av1_utils.h"
#include "box_reader_impl.h"
#include <cstring> // strcmp

#include <iostream>
#include <string>

bool checkRuleSection(const SpecDesc &spec, const char *section, Box const &root);
std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc);

namespace {

  struct ResolutionDetails {
    bool valid{false};
    int64_t width{0};
    int64_t height{0};
  };

  ResolutionDetails getOBUDetails(Box const &root, IReport */*//Erik: out*/) {
    BoxReader br;

    auto mdats = findBoxes(root, FOURCC("mdat"));
    if (mdats.size() != 1) {
      //out->error("%d mdat found, expected 1", mdats.size());
      return {false};
    }
    br.br = {mdats[0]->original + 8, (int)mdats[0]->size - 8};

    if (br.br.size < 2) {
      //out->error("Not enough bytes(=%llu) to contain an OBU", br.br.size);
      return {false};
    }

    Av1State stateUnused;
    auto obuType = 0;
    while (obuType != OBU_SEQUENCE_HEADER) { obuType = parseAv1Obus(&br, stateUnused, false); }

    if (obuType != OBU_SEQUENCE_HEADER) {
      //out->error("No OBU Sequence header found");
      return {false};
    }

    auto obuWidth = 0;
    auto obuHeight = 0;

    for (auto &sym : br.myBox.syms) {
      if (std::string(sym.name) == "max_frame_width_minus_1") { obuWidth = sym.value + 1; }
      if (std::string(sym.name) == "max_frame_height_minus_1") { obuHeight = sym.value + 1; }
    }
    return {true, obuWidth, obuHeight};
  }

  ///\FIXME: Actually parse from the OBU stream
  ResolutionDetails getMaxRenderSize() {
    return {true, 2408, 1600};
  }

  ResolutionDetails getAv01Details(Box const &root) {
    auto av01Boxes = findBoxes(root, FOURCC("av01"));

    for (auto &it : av01Boxes) {
      auto av01Width = 0;
      auto av01Height = 0;
      for (auto &sym : it->syms) {
        if (std::string(sym.name) == "width") { av01Width = sym.value; }
        if (std::string(sym.name) == "height") { av01Height = sym.value; }
      }

      return {true, av01Width, av01Height};
    }
    return {false};
  }

  bool parseAv1Configs(Box const &root, IReport *out, uint32_t trackId, const Box &minfChild,
                       Av1State &bsState, Av1State &av1cState, AV1CodecConfigurationRecord &av1cRef,
                      bool displayParsingErrors, bool obuSizeShallBeOne, bool timingInfoShoudBeZero) {
    bool av1BsFound = false, av1cFound = false, configOBUsFound = false;

    for(auto& stblChild : minfChild.children)
    {
      if(stblChild.fourcc == FOURCC("stsd"))
      {
        for(auto& stsdChild : stblChild.children)
          if(stsdChild.fourcc == FOURCC("av01"))
            for(auto& sampleEntryChild : stsdChild.children)
              if(sampleEntryChild.fourcc == FOURCC("av1C")) {
                for(auto& sym : sampleEntryChild.syms) {
                  if(!strcmp(sym.name, "seq_profile")) {
                    if (!av1cFound) {
                      av1cRef.seq_profile = sym.value;
                      av1cFound = true;
                    } else if (!configOBUsFound) {
                      av1cState.av1c.seq_profile = sym.value;
                      configOBUsFound = true;
                    } else {
                      if (displayParsingErrors)
                        out->error("The configOBUs field SHALL contain at most one Sequence Header OBU. Found several.");
                      break; //exit after first seqHdr
                    }
                  } else if(!strncmp(sym.name, "seq_level_idx", 13)) {
                    if (!configOBUsFound)
                      av1cRef.seq_level_idx_0 = sym.value;
                    else
                      av1cState.av1c.seq_level_idx_0 = sym.value;
                  } else if(!strncmp(sym.name, "seq_tier", 8)) {
                    if (!configOBUsFound)
                      av1cRef.seq_tier_0 = sym.value;
                    else
                      av1cState.av1c.seq_tier_0 = sym.value;
                  } else if(!strcmp(sym.name, "high_bitdepth")) {
                    if (!configOBUsFound)
                      av1cRef.high_bitdepth = sym.value;
                    else
                      av1cState.av1c.high_bitdepth = sym.value;
                  } else if(!strcmp(sym.name, "twelve_bit")) {
                    if (!configOBUsFound)
                      av1cRef.twelve_bit = sym.value;
                    else
                      av1cState.av1c.twelve_bit = sym.value;
                  } else if(!strcmp(sym.name, "mono_chrome")) {
                    if (!configOBUsFound)
                      av1cRef.mono_chrome = sym.value;
                    else
                      av1cState.av1c.mono_chrome = sym.value;
                  } else if(!strcmp(sym.name, "chroma_subsampling_x")) {
                    if (!configOBUsFound)
                      av1cRef.chroma_subsampling_x = sym.value;
                    else
                      av1cState.av1c.chroma_subsampling_x = sym.value;
                  } else if(!strcmp(sym.name, "chroma_subsampling_y")) {
                    if (!configOBUsFound)
                      av1cRef.chroma_subsampling_y = sym.value;
                    else
                      av1cState.av1c.chroma_subsampling_y = sym.value;
                  } else if(!strcmp(sym.name, "chroma_sample_position")) {
                    if (!configOBUsFound)
                      av1cRef.chroma_sample_position = sym.value;
                    else
                      av1cState.av1c.chroma_sample_position = sym.value;
                  } else if(!strcmp(sym.name, "obu_has_size_field")) {
                    if (obuSizeShallBeOne && sym.value != 1)
                      out->error("The flag obu_has_size_field SHALL be set to 1");
                  } else if(!strcmp(sym.name, "timing_info_present_flag")) {
                    if (timingInfoShoudBeZero && sym.value != 0)
                      out->warning("timing_info_present_flag should be zero");
                  }
                }
              }
      }
      else if(stblChild.fourcc == FOURCC("stco") || stblChild.fourcc == FOURCC("co64"))
      {
        for(auto& sym : stblChild.syms)
          if(!strcmp(sym.name, "chunk_offset"))
          {
            BoxReader br;
            auto const probeSize = 1024;
            br.br = BitReader { root.original + sym.value, probeSize };

            while(!br.empty())
            {
              auto obuType = parseAv1Obus(&br, bsState, false);

              if(obuType == OBU_SEQUENCE_HEADER) {
                av1BsFound = true;
                break;
              }
            }

            break;
          }
      }
    }

    if (av1cFound) {
      if(!av1BsFound)
        out->error("[TrackId=%u] AV1 configuration should be present. Found in av1C(%d), in configOBUs(%d), in mdat(%d).",
          trackId, av1cFound, configOBUsFound, av1BsFound);

      if(!configOBUsFound)
        av1cState.av1c = av1cRef;

      out->covered();
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
      {"isobmff"},
      {
        {"Section 2.1\n"
          "It SHALL have the 'av01' brand among the compatible brands array of the FileTypeBox",
          [](Box const &root, IReport *out) {
            auto ftyps = findBoxes(root, FOURCC("ftyp"));

            bool foundAv01 = false;
            for (auto &ftyp : ftyps) {
              for (auto &sym : ftyp->syms) {
                if (std::string(sym.name) == "compatible_brand" && sym.value == FOURCC("av01")) {
                  foundAv01 = true;
                  break;
                }
              }
            }

            if (!foundAv01) {
              out->error("No 'av01' found in compatibleBrands");
              return;
            }

            out->covered();
          }},
        {"Section 2.1\n"
          "It SHOULD indicate a structural ISOBMFF brand among the compatible brands array of\n"
          "the FileTypeBox",
          [](Box const &root, IReport *out) {
            auto ftyps = findBoxes(root, FOURCC("ftyp"));

            bool foundStructural = false;
            for (auto &ftyp : ftyps) {
              for (auto &sym : ftyp->syms) {
                if (std::string(sym.name) == "compatible_brand") {
                  switch (sym.value) {
                  case FOURCC("isom"):
                  case FOURCC("iso2"):
                  case FOURCC("iso4"):
                  case FOURCC("iso6"): foundStructural = true; break;
                  default: break;
                  }
                }
                if (foundStructural) { break; }
              }
            }

            if (!foundStructural) { return; }

            out->covered();
          }},
        {"Section 2.1\n"
          "It SHALL contain at least one track using an AV1SampleEntry",
          [](Box const &root, IReport *out) {
            bool av01Found = false;
            auto stsdBoxes = findBoxes(root, FOURCC("stsd"));

            for (auto &stsdBox : stsdBoxes) {
              auto av01Boxes = findBoxes(*stsdBox, FOURCC("av01"));
              if (!av01Boxes.empty()) {
                av01Found = true;
                break;
              }
            }

            if (!av01Found) {
              out->error("No Av1SampleEntry found");
              return;
            }

            out->covered();
          }},
        {"Section 2.2.4\n"
          "The width and height fields of the VisualSampleEntry SHALL equal the values of\n"
          "max_frame_width_minus_1 + 1 and max_frame_height_minus_1 + 1 of the Sequence Header\n"
          "OBU applying to the samples associated with this sample entry.",
          [](Box const &root, IReport *out) {
            auto obuDetails = getOBUDetails(root, out);
            if (!obuDetails.valid) { return; }

            auto av01Details = getAv01Details(root);
            if (av01Details.width != obuDetails.width || av01Details.height != obuDetails.height) {
              //Erik: out->error("No match found, OBU specifiex %dx%d");
              return;
            }

            out->covered();
          }},
        {"Section 2.2.4\n"
          "The width and height in the TrackHeaderBox SHOULD equal, respectively, the maximum\n"
          "RenderWidth, called MaxRenderWidth, and the maximum RenderHeight, called\n"
          "MaxRenderHeight, of all the frames associated with this sample entry",
          [](Box const &root, IReport *out) {
            auto obuDetails = getOBUDetails(root, out);
            if (!obuDetails.valid) { return; }

            auto trakBoxes = findBoxes(root, FOURCC("trak"));
            for (auto &trakBox : trakBoxes) {
              auto av01Details = getAv01Details(*trakBox);
              if (!av01Details.valid) { continue; }

              ResolutionDetails maxRenderSize = getMaxRenderSize();

              if (!maxRenderSize.valid) {
                out->error("No maximumRenderSize could be parsed");
                return;
              }

              auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
              if (tkhdBoxes.size() != 1) {
                out->error("%d 'tkhd' boxes found, when 1 is expected", tkhdBoxes.size());
                return;
              }

              ResolutionDetails tkhdResolution{false};
              for (auto &sym : tkhdBoxes[0]->syms) {
                if (std::string(sym.name) == "width") { tkhdResolution.width = sym.value; }
                if (std::string(sym.name) == "height") { tkhdResolution.height = sym.value; }
              }
              tkhdResolution.valid = tkhdResolution.width && tkhdResolution.height;

              if (maxRenderSize.width != obuDetails.width || maxRenderSize.height != obuDetails.height) {
                out->warning(
                    "MaxRenderWidth and/or MaxRenderHeight do not correspond to tkhd box");
                return;
              }
            }

            out->covered();
          }},
        {"Section 2.2.4\n"
          "Additionally, if MaxRenderWidth and MaxRenderHeight values do not equal respectively\n"
          "the max_frame_width_minus_1 + 1 and max_frame_height_minus_1 + 1 values of the\n"
          "Sequence Header OBU, a PixelAspectRatioBox box SHALL be present in the sample entry",
          [](Box const &root, IReport *out) {
            auto obuDetails = getOBUDetails(root, out);
            if (!obuDetails.valid) { return; }

            auto trakBoxes = findBoxes(root, FOURCC("trak"));
            for (auto &trakBox : trakBoxes) {
              auto av01Details = getAv01Details(*trakBox);
              if (!av01Details.valid) { continue; }

              ResolutionDetails maxRenderSize = getMaxRenderSize();

              if (!maxRenderSize.valid) {
                out->error("No maximumRenderSize could be parsed");
                return;
              }

              bool expectPixelAspectRatio =
                  (maxRenderSize.width != obuDetails.width || maxRenderSize.height != obuDetails.height);

              if (!expectPixelAspectRatio) { continue; }

              out->covered();

              auto paspBoxes = findBoxes(*trakBox, FOURCC("pasp"));
              if (paspBoxes.size() != 1) {
                out->error("%d 'pasp' boxes found, when 1 is expected", paspBoxes.size());
                return;
              }

              auto hSpacing = 0;
              auto vSpacing = 0;

              for (auto &sym : paspBoxes[0]->syms) {
                if (std::string(sym.name) == "hSpacing") { hSpacing = sym.value; }
                if (std::string(sym.name) == "vSpacing") { vSpacing = sym.value; }
              }

              double paspRatio = (double)hSpacing / vSpacing;
              double frameRatio = (double)(maxRenderSize.width * obuDetails.width) /
                                  (maxRenderSize.height * obuDetails.height);

              bool validPASP = (paspRatio == frameRatio);

              if (!validPASP) {
                out->error("Invalid pasp; %u / %u != %u / %u", hSpacing, vSpacing,
                          (maxRenderSize.width * obuDetails.width),
                          (maxRenderSize.height * obuDetails.height));
                return;
              }
            }
          }},
        {"Section 2.2.4\n"
          "The config field SHALL contain an AV1CodecConfigurationBox that applies to the samples\n"
          "associated with this sample entry.",
          [](Box const &root, IReport *out) {
            auto obuDetails = getOBUDetails(root, out);
            if (!obuDetails.valid) { return; }

            auto trakBoxes = findBoxes(root, FOURCC("trak"));
            for (auto &trakBox : trakBoxes) {
              auto av01Boxes = findBoxes(*trakBox, FOURCC("av01"));
              for (auto &av01Box : av01Boxes) {
                auto av1CBoxes = findBoxes(*av01Box, FOURCC("av1C"));
                if (av1CBoxes.size() != 1) {
                  out->error("%d AV1CodecConfiguration box(es) found, expected 1", av1CBoxes.size());
                  return;
                }
                if (!av1CBoxes.empty())
                  out->covered();
              }
            }
          }},
        {"Section 2.3.4\n"
          "The AV1CodecConfigurationRecord marker field SHALL be set to 1",
          [](Box const &root, IReport *out) {
            auto obuDetails = getOBUDetails(root, out);
            if (!obuDetails.valid) { return; }

            auto trakBoxes = findBoxes(root, FOURCC("trak"));
            for (auto &trakBox : trakBoxes) {
              auto av1CBoxes = findBoxes(*trakBox, FOURCC("av1C"));
              for (auto &av1CBox : av1CBoxes) {
                for (auto &sym : av1CBox->syms) {
                  //Erik: std::cerr << sym.name << ": " << sym.value << std::endl;
                  if (std::string(sym.name) == "marker" && sym.value != 1) {
                    out->error("Marker SHALL be set to 1, found %d", sym.value);
                  }
                  out->covered();
                }
              }
            }
          }},
        {"Section 2.3.4\n"
          "The AV1CodecConfigurationRecord version field SHALL be set to 1",
          [](Box const &root, IReport *out) {
            auto obuDetails = getOBUDetails(root, out);
            if (!obuDetails.valid) { return; }

            auto trakBoxes = findBoxes(root, FOURCC("trak"));
            for (auto &trakBox : trakBoxes) {
              auto av1CBoxes = findBoxes(*trakBox, FOURCC("av1C"));
              for (auto &av1CBox : av1CBoxes) {
                for (auto &sym : av1CBox->syms) {
                  //Erik: std::cerr << sym.name << ": " << sym.value << std::endl;
                  if (std::string(sym.name) == "version" && sym.value != 1) {
                    out->error("Version SHALL be set to 1, found %d", sym.value);
                  }
                  out->covered();
                }
              }
            }
          }},
        {"Section 2.3.4\n"
          "The seq_profile field indicates the AV1 profile and SHALL be equal to\n"
          "the seq_profile value from the Sequence Header OBU.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              if (parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                                if(av1cRef.seq_profile != bsState.av1c.seq_profile)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox seq_profile field value (%lld) SHALL be\n"
                                              "equal to the seq_profile value from the first Sequence Header OBU in the mdat (%lld)",
                                            trackId, av1cRef.seq_profile, bsState.av1c.seq_profile);

                                if(av1cRef.seq_profile != av1cState.av1c.seq_profile)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox seq_profile field value (%lld) SHALL be\n"
                                              "equal to the seq_profile value from the first Sequence Header OBU in configOBUS (%lld)",
                                            trackId, av1cRef.seq_profile, av1cState.av1c.seq_profile);
                              }
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The seq_level_idx_0 field indicates the value of seq_level_idx[0] found in the\n"
          "Sequence Header OBU and SHALL be equal to the value of seq_level_idx[0] in the\n"
          "Sequence Header OBU.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              if (parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                                if(av1cRef.seq_level_idx_0 != bsState.av1c.seq_level_idx_0)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox seq_level_idx_0 field value (%lld) SHALL be\n"
                                              "equal to the seq_level_idx_0 value from the first Sequence Header OBU in the mdat (%lld)",
                                            trackId, av1cRef.seq_level_idx_0, bsState.av1c.seq_level_idx_0);

                                if(av1cRef.seq_level_idx_0 != av1cState.av1c.seq_level_idx_0)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox seq_level_idx_0 field value (%lld) SHALL be\n"
                                              "equal to the seq_level_idx_0 value from the first Sequence Header OBU in configOBUS (%lld)",
                                            trackId, av1cRef.seq_level_idx_0, av1cState.av1c.seq_level_idx_0);
                              }
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The seq_tier_0 field indicates the value of seq_tier[0] found in the\n"
          "Sequence Header OBU and SHALL be equal to the value of seq_tier[0] in the\n"
          "Sequence Header OBU.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              if (parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                                if(av1cRef.seq_tier_0 != bsState.av1c.seq_tier_0)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox seq_tier_0 field value (%lld) SHALL be\n"
                                              "equal to the seq_tier_0 value from the first Sequence Header OBU in the mdat (%lld)",
                                            trackId, av1cRef.seq_tier_0, bsState.av1c.seq_tier_0);

                                if(av1cRef.seq_tier_0 != av1cState.av1c.seq_tier_0)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox seq_tier_0 field value (%lld) SHALL be\n"
                                              "equal to the seq_tier_0 value from the first Sequence Header OBU in configOBUS (%lld)",
                                            trackId, av1cRef.seq_tier_0, av1cState.av1c.seq_tier_0);
                              }
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The high_bitdepth field indicates the value of the high_bitdepth flag from the\n"
          "Sequence Header OBU.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              if (parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                                if(av1cRef.high_bitdepth != bsState.av1c.high_bitdepth)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox high_bitdepth field value (%lld) SHALL be\n"
                                              "equal to the high_bitdepth value from the first Sequence Header OBU in the mdat (%lld)",
                                            trackId, av1cRef.high_bitdepth, bsState.av1c.high_bitdepth);

                                if(av1cRef.high_bitdepth != av1cState.av1c.high_bitdepth)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox high_bitdepth field value (%lld) SHALL be\n"
                                              "equal to the high_bitdepth value from the first Sequence Header OBU in configOBUS (%lld)",
                                            trackId, av1cRef.high_bitdepth, av1cState.av1c.high_bitdepth);
                              }
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The twelve_bit field indicates the value of the twelve_bit flag from the\n"
          "Sequence Header OBU. When twelve_bit is not present in the Sequence Header\n"
          "OBU the AV1CodecConfigurationRecord twelve_bit value SHALL be 0.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              if (parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                                if(av1cRef.twelve_bit != bsState.av1c.twelve_bit)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox twelve_bit field value (%lld) SHALL be\n"
                                              "equal to the twelve_bit value from the first Sequence Header OBU in the mdat (%lld)",
                                            trackId, av1cRef.twelve_bit, bsState.av1c.twelve_bit);

                                if(av1cRef.twelve_bit != av1cState.av1c.twelve_bit)
                                  out->error("[TrackId=%u] The AV1CodecConfigurationBox twelve_bit field value (%lld) SHALL be\n"
                                              "equal to the twelve_bit value from the first Sequence Header OBU in configOBUS (%lld)",
                                            trackId, av1cRef.twelve_bit, av1cState.av1c.twelve_bit);
                              }
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The configOBUs field SHALL contain at most one Sequence Header OBU and if present, it SHALL be the first OBU.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, true, false, false);
                            }
                }
        }},
        {"Section 2.3.4\n"
          "ConfigOBUs: the flag obu_has_size_field SHALL be set to 1.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, true, false);
                            }
                }
        }},
        {"Section 2.3.4\n"
          "When a Sequence Header OBU is contained within the configOBUs of the\n"
          "AV1CodecConfigurationRecord, the values present in the Sequence Header OBU\n"
          "contained within configOBUs SHALL match the values of the\n"
          "AV1CodecConfigurationRecord.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              if (parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, false)) {
                                if(memcmp(&av1cState.av1c, &av1cRef, sizeof(AV1CodecConfigurationRecord)))
                                  out->error("[TrackId=%u] The values of the AV1CodecConfigurationBox shall match\n"
                                            "the Sequence Header OBU in the AV1 Image Item Data:\n"
                                            "\tAV1CodecConfigurationBox:\n%s\n"
                                            "\tSequence Header OBU in the AV1 Image Item Data:\n%s\n",
                                            trackId, av1cRef.toString().c_str(), av1cState.av1c.toString().c_str());
                              }
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The timing_info_present_flag in the Sequence Header OBU (in the configOBUs field or in the associated samples) SHOULD be set to 0.",
        [] (Box const& root, IReport* out) {
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
                            if(minfChild.fourcc == FOURCC("stbl")) {
                              Av1State bsState, av1cState;
                              AV1CodecConfigurationRecord av1cRef {};
                              parseAv1Configs(root, out, trackId, minfChild, bsState, av1cState, av1cRef, false, false, true);
                            }
                }
        }},
        {"Section 2.3.4\n"
          "The sample entry SHOULD contain a colr box with a colour_type set to nclx.\n"
          "If present, the values of colour_primaries, transfer_characteristics, and\n"
          "matrix_coefficients SHALL match the values given in the Sequence Header OBU (in\n"
          "the configOBUs field or in the associated samples) if the\n"
          "color_description_present_flag is set to 1. Similarly, the full_range_flag in\n"
          "the colr box shall match the color_range flag in the Sequence Header OBU. When\n"
          "configOBUs does not contain a Sequence Header OBU, this box with colour_type set\n"
          "to nclx SHALL be present.",
        [] (Box const& /*root*/, IReport* /*out*/) {
        }},
        {"Section 2.3.4\n"
          "The CleanApertureBox clap SHOULD not be present.",
        [] (Box const& /*root*/, IReport* /*out*/) {
        }},
        {"Section 2.3.4\n"
          "For sample entries corresponding to HDR content, the\n"
          "MasteringDisplayColourVolumeBox mdcv and ContentLightLevelBox clli SHOULD be\n"
          "present, and their values SHALL match the values of contained in the Metadata\n"
          "OBUs of type METADATA_TYPE_HDR_CLL and METADATA_TYPE_HDR_MDCV, if present (in\n"
          "the configOBUs or in the samples).",
        [] (Box const& /*root*/, IReport* /*out*/) {
        }},
      },
      isIsobmff,
  };

  static auto const registered = registerSpec(&specAv1ISOBMFF);
} // namespace
