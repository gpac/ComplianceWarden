#include "spec.h"
#include "fourcc.h"
#include <cstring> // strcmp
#include <sstream>

bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root)
{
  for(auto& rule : spec.rules)
  {
    std::stringstream ss(rule.caption);
    std::string line;
    std::getline(ss, line);
    std::stringstream ssl(line);
    std::string word;
    ssl >> word;

    if(word != "Section")
      throw std::runtime_error("Rule caption is misformed.");

    ssl >> word;

    if(word.rfind(section, 0) == 0)
    {
      struct Report : IReport
      {
        void error(const char*, ...) override
        {
          ++errorCount;
        }

        void warning(const char*, ...) override
        {
          /*ignored*/
        }

        int errorCount = 0;
      };
      Report r;
      rule.check(root, &r);

      if(r.errorCount)
        return false;
    }
  }

  return true;
}

std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc)
{
  std::vector<const Box*> res;

  if(root.fourcc == fourcc)
    res.push_back(&root);

  for(auto& box : root.children)
  {
    if(box.fourcc == fourcc)
    {
      res.push_back(&box);
    }
    else
    {
      auto b = findBoxes(box, fourcc);
      res.insert(res.end(), b.begin(), b.end());
    }
  }

  return res;
}

void checkEssential(Box const& root, IReport* out, uint32_t fourcc)
{
  std::vector<uint32_t> properties { 0 }; // index is 1-based

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco"))
              for(auto& ipcoChild : iprpChild.children)
                properties.push_back(ipcoChild.fourcc);

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto& iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipma"))
            {
              bool essential = false;
              uint32_t itemId = 0;

              for(auto& sym : iprpChild.syms)
              {
                if(!strcmp(sym.name, "item_ID"))
                  itemId = sym.value;
                else if(!strcmp(sym.name, "essential"))
                  essential = sym.value;
                else if(!strcmp(sym.name, "property_index"))
                {
                  if(sym.value > (int64_t)properties.size())
                  {
                    out->error("property_index \"%lld\" doesn't exist (%u detected).", sym.value, properties.size());
                    break;
                  }

                  if(properties[sym.value] == fourcc)
                    if(!essential)
                      out->error("Transformative property \"%s\" shall be marked as essential (item_ID=%u)", toString(properties[sym.value]).c_str(), itemId);
                }
              }
            }
}

std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> getItemDataOffsets(Box const& root, IReport* out, uint32_t itemID)
{
  struct ItemLocation
  {
    int construction_method = 0, data_reference_index = 0;
    int64_t base_offset = 0;
    std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> extents;

    int computeOffset(IReport* out)
    {
      if(construction_method > 1 || extents.size() > 1)
      {
        out->warning("construction_method > 1 not supported");
        return 0;
      }

      if(extents.size() > 1)
      {
        out->warning("iloc with several extensions not supported");
        return 0;
      }

      if(data_reference_index > 0)
      {
        out->warning("data_reference_index > 0 not supported");
        return 0;
      }

      int originOffset = base_offset;

      if(!extents.empty())
        originOffset += extents[0].first;

      return originOffset;
    }
  };

  std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> spans;

  std::vector<ItemLocation> itemLocs;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iloc"))
        {
          int64_t currOffset = 0;

          for(auto& sym : metaChild.syms)
          {
            if(!strcmp(sym.name, "item_ID"))
            {
              if(sym.value != itemID)
              {
                if(itemLocs.empty())
                  continue;
                else
                  break;
              }

              itemLocs.resize(itemLocs.size() + 1);

              continue;
            }

            if(itemLocs.empty())
              continue;

            auto& itemLoc = itemLocs.back();

            if(!strcmp(sym.name, "construction_method"))
              itemLoc.construction_method = sym.value;

            if(!strcmp(sym.name, "data_reference_index"))
              itemLoc.data_reference_index = sym.value;

            if(!strcmp(sym.name, "base_offset"))
              itemLoc.base_offset = sym.value;

            if(!strcmp(sym.name, "extent_offset"))
              currOffset = sym.value;

            if(!strcmp(sym.name, "extent_length"))
              itemLoc.extents.push_back({ currOffset, sym.value });
          }
        }

  for(auto& itemLoc : itemLocs)
    spans.push_back({ itemLoc.computeOffset(out), itemLoc.extents.empty() ? 0 : itemLoc.extents[0].second }); // we assume no idat-based check (construction_method = 1)

  return spans;
}

std::vector<RuleDesc> concatRules(const std::initializer_list<const std::initializer_list<RuleDesc>>& rules)
{
  std::vector<RuleDesc> v;

  for(auto& r : rules)
    v.insert(v.end(), r.begin(), r.end());

  return v;
}

