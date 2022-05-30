#include "spec_av1_utils.h"
#include "spec.h" // IReport
#include "fourcc.h"
#include <cstring> // strcmp

std::vector<uint32_t /*itemId*/> findImageItems(Box const& root, uint32_t fourcc);
std::vector<const Box*> findBoxesWithProperty(Box const& root, uint32_t itemId, uint32_t fourcc);

std::vector<std::pair<uint32_t /*ItemId*/, std::string>> getAv1ItemColorspaces(Box const& root, IReport* out)
{
  std::vector<std::pair<uint32_t /*ItemId*/, std::string>> ret;

  auto const av1ImageItemIDs = findImageItems(root, FOURCC("av01"));

  for(auto itemId : av1ImageItemIDs)
  {
    AV1CodecConfigurationRecord av1cRef {};

    auto av1Cs = findBoxesWithProperty(root, itemId, FOURCC("av1C"));

    if(av1Cs.empty())
    {
      // out->error("[ItemId=%u] No av1C configuration found (expected 1)", itemId);
      continue;
    }
    else if(av1Cs.size() > 1)
      out->error("[ItemId=%u] Found %d av1C (expected 1) - for conformance, only the first associated av1C will be considered", itemId, (int)av1Cs.size());

    auto av1C = av1Cs[0];

    for(auto& sym : av1C->syms)
    {
      if(!strcmp(sym.name, "monochrome"))
        av1cRef.mono_chrome = sym.value;

      if(!strcmp(sym.name, "chroma_subsampling_x"))
        av1cRef.chroma_subsampling_x = sym.value;

      if(!strcmp(sym.name, "chroma_subsampling_y"))
        av1cRef.chroma_subsampling_y = sym.value;

      if(!strcmp(sym.name, "chroma_sample_position"))
        av1cRef.chroma_sample_position = sym.value;
    }

    if(av1cRef.chroma_subsampling_x == 0 && av1cRef.chroma_subsampling_y == 0 && av1cRef.mono_chrome == 0)
      ret.push_back({ itemId, "YUV 4:4:4" });
    else if(av1cRef.chroma_subsampling_x == 1 && av1cRef.chroma_subsampling_y == 0 && av1cRef.mono_chrome == 0)
      ret.push_back({ itemId, "YUV 4:2:2" });
    else if(av1cRef.chroma_subsampling_x == 1 && av1cRef.chroma_subsampling_y == 1 && av1cRef.mono_chrome == 0)
      ret.push_back({ itemId, "YUV 4:2:0" });
    else if(av1cRef.chroma_subsampling_x == 1 && av1cRef.chroma_subsampling_y == 1 && av1cRef.mono_chrome == 1)
      ret.push_back({ itemId, "Monochrome 4:0:0" });
    else
      out->error("[ItemId=%u] Inconsistent AV1 colorspace: chroma_subsampling_x=%lld chroma_subsampling_y=%lld mono_chrome=%lld",
                 itemId, av1cRef.chroma_subsampling_x, av1cRef.chroma_subsampling_y, av1cRef.mono_chrome);
  }

  return ret;
}

