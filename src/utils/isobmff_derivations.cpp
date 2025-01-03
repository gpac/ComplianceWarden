#include "isobmff_derivations.h"
// #include <algorithm>
#include <cstring> // strcmp

bool DerivationGraph::visit(
  uint32_t itemIdSrc, std::list<uint32_t> &visited, std::function<void(const std::list<uint32_t> &)> onError,
  std::function<void(const std::list<uint32_t> &)> onTerminal)
{
  auto const maxDerivations = 16;
  int newVisits = 0;
  visited.push_back(itemIdSrc);

  for(auto &c : connections) {
    if(c.src == itemIdSrc) {
      if(
        c.src == c.dst || visited.size() > maxDerivations // cycles
        || !visit(c.dst, visited, onError, onTerminal)) {
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

std::string DerivationGraph::display(const std::list<uint32_t> &visited)
{
  std::string str, sep = " -> ";

  for(auto rit = visited.rbegin(); rit != visited.rend(); ++rit)
    str += itemTypes[*rit] + " (" + std::to_string(*rit) + ")" + sep;

  str.erase(str.length() - sep.length());

  return str;
}

DerivationGraph buildDerivationGraph(Box const &root)
{
  DerivationGraph graph;

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children) {
        if(metaChild.fourcc == FOURCC("iinf")) {
          for(auto &iinfChild : metaChild.children)
            if(iinfChild.fourcc == FOURCC("infe")) {
              std::string item_type;
              uint32_t item_ID = 0;

              for(auto &sym : iinfChild.syms) {
                if(!strcmp(sym.name, "item_type"))
                  item_type = toString(sym.value);
                else if(!strcmp(sym.name, "item_ID"))
                  item_ID = sym.value;
              }

              graph.itemTypes.insert({ item_ID, item_type });
            }
        } else if(metaChild.fourcc == FOURCC("iref")) {
          bool parsing = false;
          uint32_t from_item_ID = 0;

          for(auto &sym : metaChild.syms) {
            if(!strcmp(sym.name, "from_item_ID")) {
              from_item_ID = (int)sym.value;
            } else if(!strcmp(sym.name, "box_type")) {
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

Derivations getDerivationsInfo(Box const &root, uint32_t irefTypeFourcc)
{
  Derivations d;

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto &iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco")) {
              int propertyIndex = 1;

              for(auto &ipcoChild : iprpChild.children) {
                if(ipcoChild.fourcc == FOURCC("ispe")) {
                  int width = 0, height = 0;

                  for(auto &sym : ipcoChild.syms) {
                    if(!strcmp(sym.name, "image_width"))
                      width = (int)sym.value;
                    else if(!strcmp(sym.name, "image_height"))
                      height = (int)sym.value;
                  }

                  d.resolutions.insert({ propertyIndex, { width, height } });
                }

                propertyIndex++;
              }
            }

  // find 'ipma' to associate item_ID with resolution

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto &iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipma")) {
              int item_ID = 0;

              for(auto &sym : iprpChild.syms) {
                if(!strcmp(sym.name, "item_ID"))
                  item_ID = (int)sym.value;
                else if(!strcmp(sym.name, "property_index"))
                  if(d.resolutions.find(sym.value) != d.resolutions.end())
                    d.itemRes.insert({ item_ID, d.resolutions[sym.value] });
              }
            }

  // find 'iref' to find thumbnails

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iref")) {
          bool parsing = false;
          uint32_t fromItemId = 0;

          for(auto &sym : metaChild.syms) {
            if(!strcmp(sym.name, "box_type")) {
              parsing = false;

              if(sym.value == irefTypeFourcc)
                parsing = true;
            }

            if(parsing) {
              if(!strcmp(sym.name, "from_item_ID")) {
                auto it = d.itemRefs.find(sym.value);

                if(it == d.itemRefs.end()) {
                  d.itemRefs.insert({ (uint32_t)sym.value, {} });
                  it = d.itemRefs.find(sym.value);
                }

                fromItemId = sym.value;
              } else if(!strcmp(sym.name, "to_item_ID")) {
                auto it = d.itemRefs.find(fromItemId);
                it->second.push_back(sym.value);
              }
            }
          }
        }

  return d;
}
