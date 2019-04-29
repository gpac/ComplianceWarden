#include "spec.h"

template<typename H, typename P>
static bool canFind(H& haystack, P const& predicate)
{
  for(auto& needle : haystack)
    if(predicate(needle))
      return true;

  return false;
}

static const SpecDesc g_dummySpec =
{
  "Dummy Spec, v1.2",
  {
    {
      "'ftyp' box must appear first",
      [] (Box const& root, IReport* out)
      {
        if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
          out->error("first box must be ftyp");
      }
    },

    {
      "'moov' box must exist",
      [] (Box const& root, IReport* out)
      {
        if(!canFind(root.children, [] (Box const& box) { return box.fourcc == FOURCC("moov"); }))
          out->error("moov box not found");
      }
    },
  },
};

static auto registered = registerSpec(&g_dummySpec);

