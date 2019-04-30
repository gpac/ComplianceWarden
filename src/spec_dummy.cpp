#include "spec.h"

template<typename H, typename P>
static bool canFind(H& haystack, P const& predicate)
{
  for(auto& needle : haystack)
    if(predicate(needle))
      return true;

  return false;
}

namespace
{
void parseDumy(IReader* br)
{
  br->sym("holy", 32);
}

ParseBoxFunc* getParseFunction(uint32_t fourcc)
{
  switch(fourcc)
  {
  case FOURCC("dumy"):
    return &parseDumy;
  default:
    return nullptr;
  }
}
}

static const SpecDesc spec =
{
  "dummy",
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

    {
      "'holy' symbol in 'dumy' box must not be equal to 666",
      [] (Box const& root, IReport* out)
      {
        Box const* pDumy = nullptr;

        for(auto& child : root.children)
          if(child.fourcc == FOURCC("dumy"))
            pDumy = &child;

        if(!pDumy)
          return; // nothing to check

        if(!pDumy->syms.empty() && pDumy->syms[0].value == 666)
          out->error("'holy' symbol in 'dumy' box is equal to 666");
      }
    },
  },
  &getParseFunction,
};

static auto const registered = registerSpec(&spec);

