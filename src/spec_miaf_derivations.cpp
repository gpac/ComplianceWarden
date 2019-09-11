#include "spec.h"
#include "fourcc.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <list>
#include <map>

struct DerivationGraph
{
  // returns false on cyclic
  bool visit(uint32_t itemIdSrc, std::list<uint32_t> visited, std::function<void(std::list<uint32_t> &)> onError, std::function<void(std::list<uint32_t> &)> onTerminal)
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

  struct Connection
  {
    uint32_t src, dst;
  };

  std::vector<Connection> connections;
  std::map<uint32_t /*item_ID*/, std::string /*item_type*/> itemTypes;
};

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
                graph.connections.push_back({ from_item_ID, (uint32_t)sym.value });
          }
        }
      }

  return graph;
}

const std::initializer_list<RuleDesc> getRulesDerivations()
{
  static const
  std::initializer_list<RuleDesc> rulesDerivations =
  {
    {
      "Section 7.3.9\n"
      "An identity derivation shall not be derived immediately from another identity\n"
      "derivation",
      [] (Box const& root, IReport* out)
      {
        auto graph = buildDerivationGraph(root);

        auto check = [&] (std::list<uint32_t>& visited) {
            std::string last;

            for(auto v : visited)
            {
              if(graph.itemTypes[v] == "iden" && last == "iden")
                out->error("An identity derivation shall not be derived immediately from another identity (item_ID=%u)", v);

              last = graph.itemTypes[v];
            }
          };

        auto onError = [&] (std::list<uint32_t>& visited) {
            std::string str = "init";

            for(auto v : visited)
              str += " -> " + graph.itemTypes[v] + " (" + std::to_string(v) + ")";

            out->error("Detected error in derivations: %s", str.c_str());
          };

        for(auto& c : graph.connections)
        {
          std::list<uint32_t> visited;

          if(!graph.visit(c.src, visited, onError, check))
            out->error("Detected cycle in derivations.");
        }
      }
    },
    {
      "Section 7.3.11.1\n"
      "If derivations occur, they shall be in this order:\n"
      "- mandatory coded image(s)\n"
      "- optional identity derivation (subclause 7.3.11.2)\n"
      "- optional grid (subclause 7.3.11.4)\n"
      "- optional identity derivation (subclause 7.3.11.2)\n"
      "- optional overlay (subclause 7.3.11.3)\n"
      "- optional identity derivation (subclause 7.3.11.2)",
      [] (Box const& root, IReport* out)
      {
        auto graph = buildDerivationGraph(root);

        auto check = [&] (std::list<uint32_t>& visited) {
            std::vector<std::string> expected = { "iden", "grid", "iden", "iovl", "iden" };

            if(visited.size() > expected.size())
            {
              out->error("Too many derivations: %d (expected %d)", visited.size(), expected.size());
              return;
            }

            auto visitedIt = visited.begin();

            for(int i = 0; i < (int)visited.size(); ++i, ++visitedIt)
              if(graph.itemTypes[*visitedIt] != expected[i])
                out->error("Wrong derivation at index %d: expected \"%s\", got \"%s\"", i, expected[i].c_str(), graph.itemTypes[*visitedIt].c_str());
          };

        auto onError = [&] (std::list<uint32_t>& visited) {
            std::string str = "init";

            for(auto v : visited)
              str += " -> " + graph.itemTypes[v] + " (" + std::to_string(v) + ")";

            out->error("Detected error in derivations: %s", str.c_str());
          };

        for(auto& c : graph.connections)
        {
          std::list<uint32_t> visited;

          if(!graph.visit(c.src, visited, onError, check))
            out->error("Detected cycle in derivations.");
        }
      }
    },
  };
  return rulesDerivations;
}

