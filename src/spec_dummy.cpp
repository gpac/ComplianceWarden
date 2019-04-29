#include "spec.h"

extern const SpecDesc g_dummySpec =
{
  {
    {
      "'ftyp' box must appear first",
      [] (Box const& root, IOutput* out)
      {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
          out->error("first box must be ftyp");
      }
    },

    {
      "'moov' box must exist",
      [] (Box const& root, IOutput* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            found = true;

        if(!found)
          out->error("moov box not found");
      }
    },
  },
};

