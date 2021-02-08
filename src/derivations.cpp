#include "derivations.h"
#include <algorithm>
#include <cstring>

bool DerivationGraph::visit(uint32_t itemIdSrc, std::list<uint32_t> visited, std::function<void(const std::list<uint32_t> &)> onError, std::function<void(const std::list<uint32_t> &)> onTerminal)
{
  int newVisits = 0;
  visited.push_back(itemIdSrc);

  for(auto& c : connections)
  {
    if(c.src == itemIdSrc)
    {
      if(std::find(visited.begin(), visited.end(), c.dst) != visited.end()
         || !visit(c.dst, visited, onError, onTerminal))
      {
        visited.push_back(c.dst);
        onError(visited);
        return false;
      }

      newVisits++;
    }
  }

  if(newVisits == 0)
    onTerminal(visited);

  return true;
}

std::string DerivationGraph::display(const std::list<uint32_t>& visited)
{
  std::string str, sep = " -> ";

  for(auto rit = visited.rbegin(); rit != visited.rend(); ++rit)
    str += itemTypes[*rit] + " (" + std::to_string(*rit) + ")" + sep;

  str.erase(str.length() - sep.length());

  return str;
}

DerivationGraph buildDerivationGraph(Box const& root)
{
  DerivationGraph graph;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
      {
        if(metaChild.fourcc == FOURCC("iinf"))
        {
          for(auto& iinfChild : metaChild.children)
            if(iinfChild.fourcc == FOURCC("infe"))
            {
              std::string item_type;
              uint32_t item_ID = 0;

              for(auto& sym : iinfChild.syms)
              {
                if(!strcmp(sym.name, "item_type"))
                  item_type = toString(sym.value);
                else if(!strcmp(sym.name, "item_ID"))
                  item_ID = sym.value;
              }

              graph.itemTypes.insert({ item_ID, item_type });
            }
        }
        else if(metaChild.fourcc == FOURCC("iref"))
        {
          bool parsing = false;
          uint32_t from_item_ID = 0;

          for(auto& sym : metaChild.syms)
          {
            if(!strcmp(sym.name, "from_item_ID"))
            {
              from_item_ID = (int)sym.value;
            }
            else if(!strcmp(sym.name, "box_type"))
            {
              if(sym.value == FOURCC("dimg"))
                parsing = true;
              else
                parsing = false;
            }

            if(parsing)
              if(!strcmp(sym.name, "to_item_ID"))
                graph.connections.push_back({ (uint32_t)sym.value, from_item_ID });
          }
        }
      }

  return graph;
}

