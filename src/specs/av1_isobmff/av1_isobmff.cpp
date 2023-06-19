#include "av1_utils.h"
#include "box_reader_impl.h"
#include <cstring> // strcmp

#include <iostream>

bool checkRuleSection(const SpecDesc &spec, const char *section, Box const &root);
std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc);

namespace {
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
           "It SHOULD indicate a structural ISOBMFF brand among the compatible brands array of the "
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
             auto av01Boxes = findBoxes(root, FOURCC("av01"));

             if (av01Boxes.empty()) {
               out->error("No Av1SampleEntry found");
               return;
             }

             out->covered();
           }},
      },
      nullptr,
  };

  static auto const registered = registerSpec(&specAv1ISOBMFF);
} // namespace
