#include "spec.h"
#include "fourcc.h"

extern void checkEssential(Box const& root, IReport* out, uint32_t fourcc);

static const SpecDesc spec =
{
  "avif",
  "AVIF v1.0.0, 19 February 2019\n"
  "https://aomediacodec.github.io/av1-avif/",
  { "heif" },
  {
    {
      "Section 2.2.1\n"
      "AV1 Item Configuration Property [...] shall be marked as essential.",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("av1C"));
      }
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

