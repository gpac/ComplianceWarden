#include "spec.h"

template<typename H, typename P>
static bool canFind(H& haystack, P const& predicate)
{
  for(auto& needle : haystack)
    if(predicate(needle))
      return true;

  return false;
}

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
        if(!canFind(root.children, [] (Box const& box) { return box.fourcc == FOURCC("moov"); }))
          out->error("moov box not found");
      }
    },
  },
};

