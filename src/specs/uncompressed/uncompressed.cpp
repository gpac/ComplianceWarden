#include <algorithm> // std::find
#include <cassert>
#include <cstring>
#include <functional>
#include <map>

#include "fourcc.h"
#include "spec.h"

static const SpecDesc specUncompressed = {
  "uncompressed",
  "Uncompressed  - ISO/IEC 23001-17 - FDIS",
  { "heif" },
  {
    { 
      "Section 4.2\n"
      "The uncompressed video sample entry shall contain one UncompressedFrameConfigBox\n",
      [](Box const &root, IReport *out) {
        (void) root;
        (void) out;
      } },

    { 
      "Section 4.2\n"
      "The uncompressed video sample entry shall contain one ComponentDefinitionBox\n"
      "if the UncompressedFrameConfigBox version is not 1\n",
      [](Box const &root, IReport *out) {
        (void) root;
        (void) out;
     } },
  },
  isIsobmff,
};

static auto const registered = registerSpec(&specUncompressed);
