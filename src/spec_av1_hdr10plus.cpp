#include "spec.h"

static const SpecDesc specAv1Hdr10plus =
{
  "av1hdr10plus",
  "HDR10+ AV1 Metadata Handling Specification, 8 December 2021\n"
  "https://aomediacodec.github.io/av1-hdr10plus/",
  { "isobmff" },
  {
    {
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&specAv1Hdr10plus);
