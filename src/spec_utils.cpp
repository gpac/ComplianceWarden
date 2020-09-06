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

