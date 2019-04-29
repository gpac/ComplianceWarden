#include "spec.h"

static const SpecDesc spec =
{
  "HEIF",
  {
    {
      "This is a placeholder rule",
      [] (Box const &, IReport*)
      {
      }
    },
  },
};

static auto const registered = registerSpec(&spec);

