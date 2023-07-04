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

  ResolutionDetails getOBUDetails(Box const &root, IReport *out) {
    BoxReader br;

    auto mdats = findBoxes(root, FOURCC("mdat"));
    if (mdats.size() != 1) {
      out->error("%d mdat found, expected 1", mdats.size());
      return {false};
    }
    br.br = {mdats[0]->original + 8, (int)mdats[0]->size - 8};

    if (br.br.size < 2) {
      out->error("Not enough bytes(=%llu) to contain an OBU", br.br.size);
      return {false};
    }

    Av1State stateUnused;
    auto obuType = 0;
    while (obuType != OBU_SEQUENCE_HEADER) { obuType = parseAv1Obus(&br, stateUnused, false); }

    if (obuType != OBU_SEQUENCE_HEADER) {
      out->error("No OBU Sequence header found");
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
           "It SHOULD indicate a structural ISOBMFF brand among the compatible brands array of "
           "the "
           "FileTypeBox",
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
           "The width and height fields of the VisualSampleEntry SHALL equal the values of "
           "max_frame_width_minus_1 + 1 and max_frame_height_minus_1 + 1 of the Sequence Header "
           "OBU applying to the samples associated with this sample entry.",
           [](Box const &root, IReport *out) {
             auto obuDetails = getOBUDetails(root, out);
             if (!obuDetails.valid) { return; }

             auto av01Details = getAv01Details(root);
             if (av01Details.width != obuDetails.width || av01Details.height != obuDetails.height) {
               out->error("No match found, OBU specifiex %dx%d");
               return;
             }

             out->covered();
           }},
          {"Section 2.2.4\n"
           "Additionally, if MaxRenderWidth and MaxRenderHeight values do not equal respectively "
           "the max_frame_width_minus_1 + 1 and max_frame_height_minus_1 + 1 values of the "
           "Sequence Header OBU, a PixelAspectRatioBox box SHALL be present in the sample entry",
           [](Box const &root, IReport *out) {
             auto obuDetails = getOBUDetails(root, out);
             if (!obuDetails.valid) { return; }

             auto trakBoxes = findBoxes(root, FOURCC("trak"));
             for (auto &trakBox : trakBoxes) {
               auto av01Details = getAv01Details(*trakBox);
               if (!av01Details.valid) { continue; }

               ResolutionDetails tkhdDetails{false};

               auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
               for (auto &tkhdBox : tkhdBoxes) {
                 for (auto &sym : tkhdBox->syms) {
                   if (std::string(sym.name) == "width") { tkhdDetails.width = sym.value; }
                   if (std::string(sym.name) == "height") { tkhdDetails.height = sym.value; }
                 }
                 if (tkhdDetails.width && tkhdDetails.height) { tkhdDetails.valid = true; }
                 break;
               }
               if (!tkhdDetails.valid) {
                 out->error("No resolution data found in 'tkhd' box");
                 return;
               }

               bool expectPixelAspectRatio =
                   (tkhdDetails.width != obuDetails.width || tkhdDetails.height != obuDetails.height);

               if (!expectPixelAspectRatio) { continue; }

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
               double frameRatio = (double)(tkhdDetails.width * obuDetails.width) /
                                   (tkhdDetails.height * obuDetails.height);

               bool validPASP = (paspRatio == frameRatio);

               if (!validPASP) {
                 out->error("Invalid pasp; %u / %u != %u / %u", hSpacing, vSpacing,
                            (tkhdDetails.width * obuDetails.width), (tkhdDetails.height * obuDetails.height));
                 return;
               }
             }

             out->covered();
           }},
      },
      nullptr,
  };

  static auto const registered = registerSpec(&specAv1ISOBMFF);
} // namespace
