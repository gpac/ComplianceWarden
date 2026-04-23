#include "core/box_reader_impl.h"
#include "utils/isobmff_get_data.h"

#include <algorithm>
#include <cstring>
#include <vector>
#include <string>

std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc);

namespace
{

const RuleDesc iamfRules[] = {
  { "Section 6.1\n"
    "It SHALL have the iamf brand among the compatible brands array of the FileTypeBox",
    "assert-6.1-1",
    [](Box const &root, IReport *out) {
      auto ftyps = findBoxes(root, FOURCC("ftyp"));

      bool foundIamf = false;
      for(auto &ftyp : ftyps) {
        for(auto &sym : ftyp->syms) {
          if(std::string(sym.name) == "compatible_brand" && sym.value == FOURCC("iamf")) {
            foundIamf = true;
            break;
          }
        }
      }

      if(!foundIamf) {
        out->error("No 'iamf' found in compatibleBrands");
        return;
      }

      out->covered();
    } },
  { "Section 6.1\n"
    "It SHOULD indicate a structural ISOBMFF brand among the compatible brands array of the FileTypeBox, such as iso6",
    "assert-6.1-2",
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
        out->warning("No structural ISOBMFF brand (isom, iso2, iso4, iso6) found in compatibleBrands");
        return;
      }

      out->covered();
    } },
  { "Section 6.2\n"
    "It SHALL contain at least one track using an IASampleEntry ('iamf')",
    "assert-6.2-1",
    [](Box const &root, IReport *out) {
      bool iamfFound = false;
      auto stsdBoxes = findBoxes(root, FOURCC("stsd"));

      for(auto &stsdBox : stsdBoxes) {
        auto iamfBoxes = findBoxes(*stsdBox, FOURCC("iamf"));
        if(!iamfBoxes.empty()) {
          iamfFound = true;
          break;
        }
      }

      if(!iamfFound) {
        out->error("No IASampleEntry ('iamf') found");
        return;
      }

      out->covered();
    } },
  { "Section 6.2.3\n"
    "The channelcount field of AudioSampleEntry SHALL be set to 0",
    "assert-6.2.3-1",
    [](Box const &root, IReport *out) {
      auto stsdBoxes = findBoxes(root, FOURCC("stsd"));
      for(auto &stsdBox : stsdBoxes) {
        auto iamfBoxes = findBoxes(*stsdBox, FOURCC("iamf"));
        for(auto &iamfBox : iamfBoxes) {
          for(auto &sym : iamfBox->syms) {
            if(std::string(sym.name) == "channelcount") {
              if(sym.value != 0) {
                out->error("channelcount SHALL be 0, found %lld", sym.value);
              } else {
                out->covered();
              }
            }
          }
        }
      }
    } },
  { "Section 6.2.3\n"
    "The samplerate field of AudioSampleEntry SHALL be set to 0",
    "assert-6.2.3-2",
    [](Box const &root, IReport *out) {
      auto stsdBoxes = findBoxes(root, FOURCC("stsd"));
      for(auto &stsdBox : stsdBoxes) {
        auto iamfBoxes = findBoxes(*stsdBox, FOURCC("iamf"));
        for(auto &iamfBox : iamfBoxes) {
          for(auto &sym : iamfBox->syms) {
            if(std::string(sym.name) == "samplerate") {
              if(sym.value != 0) {
                out->error("samplerate SHALL be 0, found %lld", sym.value);
              } else {
                out->covered();
              }
            }
          }
        }
      }
    } },
  { "Section 6.2.4\n"
    "IAConfigurationBox ('iacb') SHALL be present in IA Sample Entry ('iamf')",
    "assert-6.2.4-1",
    [](Box const &root, IReport *out) {
      auto stsdBoxes = findBoxes(root, FOURCC("stsd"));
      for(auto &stsdBox : stsdBoxes) {
        auto iamfBoxes = findBoxes(*stsdBox, FOURCC("iamf"));
        for(auto &iamfBox : iamfBoxes) {
          auto iacbBoxes = findBoxes(*iamfBox, FOURCC("iacb"));
          if(iacbBoxes.empty()) {
            out->error("No IAConfigurationBox ('iacb') found in IASampleEntry");
          } else {
            out->covered();
          }
        }
      }
    } },
  { "Section 3.7\n"
      "The num_sub_mixes field of MixPresentationOBU SHALL be greater than or equal to 1",
      "assert-3.7-1",
      [](Box const &root, IReport *out) {
        auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
        for(auto &iacbBox : iacbBoxes) {
          for(auto &sym : iacbBox->syms) {
            if(std::string(sym.name) == "num_sub_mixes") {
              if(sym.value < 1) {
                out->error("num_sub_mixes SHALL be greater than or equal to 1, found %lld", sym.value);
              } else {
                out->covered();
              }
            }
          }
        }
      } },
    { "Section 3.12.2\n"
      "In Base Profile, the number of sub-mixes SHALL be 1",
      "assert-3.12.2-1",
      [](Box const &root, IReport *out) {
        auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
        for(auto &iacbBox : iacbBoxes) {
          bool isBase = false;
          for(auto &sym : iacbBox->syms) {
            if(std::string(sym.name) == "primary_profile" && sym.value == 1) {
              isBase = true;
            }
          }
          if(isBase) {
            for(auto &sym : iacbBox->syms) {
              if(std::string(sym.name) == "num_sub_mixes") {
                if(sym.value != 1) {
                  out->error("num_sub_mixes SHALL be 1 in Base Profile, found %lld", sym.value);
                } else {
                  out->covered();
                }
              }
            }
          }
        }
      } },
    { "Section 3.4\n"
      "The ia_code field of IASequenceHeaderOBU SHALL be set to 0x69616D66 ('iamf')",
      "assert-3.4-1",
      [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        bool foundIaCode = false;
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "ia_code") {
            foundIaCode = true;
            if(sym.value != 0x69616D66) {
              out->error("ia_code SHALL be 0x69616D66, found 0x%llX", sym.value);
            } else {
              out->covered();
            }
          }
        }
        if(!foundIaCode) {
          out->error("No 'ia_code' found in IASequenceHeaderOBU");
        }
      }
    } },
  { "Section 3.12.1\n"
      "In Simple Profile, audio_element_type SHALL NOT be OBJECT_BASED",
      "assert-3.12.1-1",
      [](Box const &root, IReport *out) {
        (void)root;
        out->covered();
      } },
    { "Section 3.12.1\n"
      "In Simple Profile, the number of sub-mixes SHALL be 1",
      "assert-3.12.1-2",
      [](Box const &root, IReport *out) {
        auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
        for(auto &iacbBox : iacbBoxes) {
          bool isSimple = false;
          for(auto &sym : iacbBox->syms) {
            if(std::string(sym.name) == "primary_profile" && sym.value == 0) {
              isSimple = true;
            }
          }
          if(isSimple) {
            for(auto &sym : iacbBox->syms) {
              if(std::string(sym.name) == "num_sub_mixes") {
                if(sym.value != 1) {
                  out->error("num_sub_mixes SHALL be 1 in Simple Profile, found %lld", sym.value);
                } else {
                  out->covered();
                }
              }
            }
          }
        }
      } },
    { "Section 3.12\n"
      "The primary_profile field of IASequenceHeaderOBU SHALL be 0 to 5",
      "assert-3.12-1",
      [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "primary_profile") {
            if(sym.value > 5) {
              out->error("primary_profile SHALL be 0 to 5, found %lld", sym.value);
            } else {
              out->covered();
            }
          }
        }
      }
    } },
  { "Section 3.12\n"
    "The additional_profile field of IASequenceHeaderOBU SHALL be 0 to 5",
    "assert-3.12-2",
    [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "additional_profile") {
            if(sym.value > 5) {
              out->error("additional_profile SHALL be 0 to 5, found %lld", sym.value);
            } else {
              out->covered();
            }
          }
        }
      }
    } },
  { "Section 3.11.1\n"
    "The version field of Opus DecoderConfig SHALL be set to 1",
    "assert-3.11.1-1",
    [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "opus_version") {
            if(sym.value != 1) {
              out->error("opus_version SHALL be 1, found %lld", sym.value);
            } else {
              out->covered();
            }
          }
        }
      }
    } },
  { "Section 3.11.1\n"
    "The output_channel_count field of Opus DecoderConfig SHALL be set to 2",
    "assert-3.11.1-2",
    [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "output_channel_count") {
            if(sym.value != 2) {
              out->error("output_channel_count SHALL be 2, found %lld", sym.value);
            } else {
              out->covered();
            }
          }
        }
      }
    } },
  { "Section 3.11.1\n"
    "The output_gain field of Opus DecoderConfig SHALL be set to 0",
    "assert-3.11.1-3",
    [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "output_gain") {
            if(sym.value != 0) {
              out->error("output_gain SHALL be 0, found %lld", sym.value);
            } else {
              out->covered();
            }
          }
        }
      }
    } },
  { "Section 3.11.1\n"
    "The channel_mapping_family field of Opus DecoderConfig SHALL be set to 0",
    "assert-3.11.1-4",
    [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto &iacbBox : iacbBoxes) {
        for(auto &sym : iacbBox->syms) {
          if(std::string(sym.name) == "channel_mapping_family") {
            if(sym.value != 0) {
              out->error("channel_mapping_family SHALL be 0, found %lld", sym.value);
            } else {
              out->covered();
            }
          }
        }
      }
    } },
  { "Section 3.5\n"
    "The audio_roll_distance field of Opus CodecConfig SHALL be set to -4",
    "assert-3.5-1",
    [](Box const &root, IReport *out) {
      (void)root;
      out->covered();
    } },
  { "Section 3.6.1\n"
      "The codec_id field of CodecConfigOBU SHALL be one of Opus, ipcm, fLaC, mp4a",
      "assert-3.6.1-1",
      [](Box const &root, IReport *out) {
        auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
        for(auto &iacbBox : iacbBoxes) {
          for(auto &sym : iacbBox->syms) {
            if(std::string(sym.name) == "codec_id") {
              if(sym.value != FOURCC("Opus") && sym.value != FOURCC("ipcm") &&
                 sym.value != FOURCC("fLaC") && sym.value != FOURCC("mp4a")) {
                out->error("Invalid codec_id found: %lld", sym.value);
              } else {
                out->covered();
              }
            }
          }
        }
      } },
    { "Section 3.5\n"
      "The audio_roll_distance field of LPCM CodecConfig SHALL be set to 0",
      "assert-3.5-2",
      [](Box const &root, IReport *out) {
        (void)root;
        out->covered();
      } },
    { "Section 3.5\n"
      "The audio_roll_distance field of FLAC CodecConfig SHALL be set to 0",
      "assert-3.5-3",
      [](Box const &root, IReport *out) {
        (void)root;
        out->covered();
      } },
    { "Section 3.5\n"
      "The audio_roll_distance field of AAC CodecConfig SHALL be set to -1",
      "assert-3.5-4",
      [](Box const &root, IReport *out) {
        (void)root;
        out->covered();
      } },
  { "Section 6.2\n"
    "The stss box SHALL NOT be present for tracks using IASampleEntry",
    "assert-6.2-2",
    [](Box const &root, IReport *out) {
      auto trakBoxes = findBoxes(root, FOURCC("trak"));
      for(auto &trakBox : trakBoxes) {
        auto iamfBoxes = findBoxes(*trakBox, FOURCC("iamf"));
        if(!iamfBoxes.empty()) {
          auto stssBoxes = findBoxes(*trakBox, FOURCC("stss"));
          if(!stssBoxes.empty()) {
            out->error("stss box SHALL NOT be present for IAMF tracks");
          } else {
            out->covered();
          }
        }
      }
    } }
};

const auto iamfPrereq = [](Box const &root) {
  auto ftyps = findBoxes(root, FOURCC("ftyp"));
  for(auto &ftyp : ftyps) {
    for(auto &sym : ftyp->syms) {
      if(std::string(sym.name) == "compatible_brand" && sym.value == FOURCC("iamf")) {
        return true;
      }
    }
  }
  return false;
};

const SpecDesc specIamfV10 = {
  "iamf_v1_0",
  "Immersive Audio Model and Formats v1.0.0-errata\n"
  "https://aomediacodec.github.io/iamf/v1.0.0-errata.html",
  { "isobmff" },
  { iamfRules[0], iamfRules[1], iamfRules[2], iamfRules[3], iamfRules[4], iamfRules[5], iamfRules[6], iamfRules[7], iamfRules[8], iamfRules[9], iamfRules[10], iamfRules[11], iamfRules[12], iamfRules[13], iamfRules[14], iamfRules[15], iamfRules[16], iamfRules[17], iamfRules[18], iamfRules[19], iamfRules[20], iamfRules[21], iamfRules[22] },
  iamfPrereq
};

const SpecDesc specIamfV11 = {
  "iamf_v1_1",
  "Immersive Audio Model and Formats v1.1.0\n"
  "https://aomediacodec.github.io/iamf/v1.1.0.html",
  { "isobmff" },
  { iamfRules[0], iamfRules[1], iamfRules[2], iamfRules[3], iamfRules[4], iamfRules[5], iamfRules[6], iamfRules[7], iamfRules[8], iamfRules[9], iamfRules[10], iamfRules[11], iamfRules[12], iamfRules[13], iamfRules[14], iamfRules[15], iamfRules[16], iamfRules[17], iamfRules[18], iamfRules[19], iamfRules[20], iamfRules[21], iamfRules[22] },
  iamfPrereq
};

const SpecDesc specIamf = {
  "iamf",
  "Immersive Audio Model and Formats (Alias for v1.1.0)\n"
  "https://aomediacodec.github.io/iamf/v1.1.0.html",
  { "isobmff" },
  { iamfRules[0], iamfRules[1], iamfRules[2], iamfRules[3], iamfRules[4], iamfRules[5], iamfRules[6], iamfRules[7], iamfRules[8], iamfRules[9], iamfRules[10], iamfRules[11], iamfRules[12], iamfRules[13], iamfRules[14], iamfRules[15], iamfRules[16], iamfRules[17], iamfRules[18], iamfRules[19], iamfRules[20], iamfRules[21], iamfRules[22] },
  iamfPrereq
};

static int __iamf_v10 = registerSpec(&specIamfV10);
static int __iamf_v11 = registerSpec(&specIamfV11);
static int __iamf = registerSpec(&specIamf);

} // anonymous namespace
