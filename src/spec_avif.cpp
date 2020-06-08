#include "spec.h"
#include "fourcc.h"

static const SpecDesc spec =
{
  "avif",
  "AVIF v1.0.0, 19 February 2019\n"
  "https://aomediacodec.github.io/av1-avif/",
  { "heif" },
  {
    {
      "'ftyp' box must appear first",
      [] (Box const& root, IReport* out)
      {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
          out->error("first box must be ftyp");
      }
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

