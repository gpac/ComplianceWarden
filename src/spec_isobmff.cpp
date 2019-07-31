#include "spec.h"

static const SpecDesc spec =
{
  "isobmff",
  "ISO Base Media File Format\n"
  "MPEG-4 part 22 - ISO/IEC 14496-12 - MPEG-4 Part 12 - m17277 (6th+FDAM1+FDAM2+COR1-R4)",
  {
    {
      "Section:\n"
      "Rule line 1\n"
      "Rule line 2",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
      },
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

