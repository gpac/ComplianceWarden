#include "spec.h"
#include "fourcc.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <list>
#include <map>
#include <set>

namespace
{
struct DerivationGraph
{
  // returns false on cyclic
  bool visit(uint32_t itemIdSrc, std::list<uint32_t> visited, std::function<void(const std::list<uint32_t> &)> onError, std::function<void(const std::list<uint32_t> &)> onTerminal)
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

          if(onError)
            onError(visited);

          return false;
        }

        newVisits++;
      }
    }

    if(newVisits == 0)
      if(onTerminal)
        onTerminal(visited);

    return true;
  }

  std::string display(const std::list<uint32_t>& visited)
  {
    std::string str, sep = " -> ";

    for(auto rit = visited.rbegin(); rit != visited.rend(); ++rit)
      str += itemTypes[*rit] + " (" + std::to_string(*rit) + ")" + sep;

    str.erase(str.length() - sep.length());

    return str;
  }

  std::vector<std::vector<uint32_t>> getDerivationChains(uint32_t rootItemId)
  {
    std::vector<std::vector<uint32_t>> derivationChains;

    for(auto& c : connections)
      if(c.src == rootItemId)
      {
        auto sub = getDerivationChains(c.dst);

        if(sub.empty())
          sub.push_back({ c.dst }); // terminal

        for(auto& derivationChain : sub)
        {
          derivationChain.insert(derivationChain.begin(), rootItemId);
          derivationChains.push_back(derivationChain);
        }
      }

    return derivationChains;
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
                graph.connections.push_back({ (uint32_t)sym.value, from_item_ID });
          }
        }
      }

  return graph;
}
}

const std::initializer_list<RuleDesc> getRulesDerivations()
{
  static const
  std::initializer_list<RuleDesc> rulesDerivations =
  {
    {
      "Section 7.3.6.7\n"
      "[Transformative properties][if used] shall be indicated to be applied in the\n"
      "following order: clean aperture first, then rotation, then mirror.\n",
      [] (Box const& root, IReport* out)
      {
        auto isTransformative = [] (uint32_t fourcc) {
            if(fourcc == FOURCC("clap") || fourcc == FOURCC("irot") || fourcc == FOURCC("imir"))
              return true;
            else
              return false;
          };

        // step 1: find transformative properties
        std::map<uint32_t /*1-based*/, uint32_t /*fourcc*/> transformativeProperties;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipco"))
                  {
                    int index = 1;

                    for(auto& ipcoChild : iprpChild.children)
                    {
                      if(isTransformative(ipcoChild.fourcc))
                        transformativeProperties[index] = ipcoChild.fourcc;

                      index++;
                    }
                  }

        // step 2: find properties per item_IDs
        std::map<uint32_t /*itemID*/, std::vector<uint32_t /*fourcc*/>> itemIdTransformativeProperties;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    int item_ID = 0;

                    for(auto& sym : iprpChild.syms)
                      if(!strcmp(sym.name, "item_ID"))
                        item_ID = (int)sym.value;
                      else if(!strcmp(sym.name, "property_index"))
                        for(auto& p : transformativeProperties)
                          if(p.first == sym.value)
                            itemIdTransformativeProperties[item_ID].push_back(p.second);
                  }

        // step 3: build derivation chain per item_ID
        std::multimap<uint32_t, std::vector<uint32_t>> derivationTransformationChains;
        {
          auto graph = buildDerivationGraph(root);

          // detect infinite recursion
          for(auto& c : graph.connections)
          {
            std::list<uint32_t> visited;

            if(!graph.visit(c.src, visited, nullptr, nullptr))
              out->error("Detected cycle in derivations. Aborting");
          }

          std::set<uint32_t /*item_ID*/> sourceIds; // definied as never dst by any ID

          {
            std::set<uint32_t> dst;

            for(auto& c : graph.connections)
              dst.insert(c.dst);

            for(auto& c : graph.connections)
              if(dst.find(c.src) == dst.end())
                sourceIds.insert(c.src);
          }

          std::multimap<uint32_t /*source item_ID*/, std::vector<uint32_t /*item_IDs, including the source*/>> sourceIdDerivationChains;

          for(auto sourceId : sourceIds)
          {
            auto derivationChains = graph.getDerivationChains(sourceId);

            for(auto& d : derivationChains)
              sourceIdDerivationChains.insert({ sourceId, d });
          }

          // transformations only
          for(auto& derivationChain : sourceIdDerivationChains)
          {
            std::vector<uint32_t> propertyFourCCs;

            for(auto& itemId : derivationChain.second)
              for(auto property : itemIdTransformativeProperties[itemId])
                propertyFourCCs.push_back(property);

            derivationTransformationChains.insert({ derivationChain.first, propertyFourCCs });
          }
        }

        // step 4: concatenate transformations and check
        const std::vector<uint32_t> expectedTProps { FOURCC("clap"), FOURCC("irot"), FOURCC("imir") };

        for(auto& derivationChain : derivationTransformationChains)
        {
          auto actualTProps = derivationChain.second;

          if(actualTProps.size() > expectedTProps.size())
          {
            out->error("Item_ID=%d: too many transformative properties (%d instead of maximum %d)", derivationChain.first, (int)actualTProps.size(), (int)expectedTProps.size());
            return;
          }

          std::string expected, actual;

          for(auto& v : expectedTProps)
            expected += toString(v) + " ";

          for(auto& v : actualTProps)
            actual += toString(v) + " ";

          for(size_t i = 0, j = 0; i < actualTProps.size() && j < expectedTProps.size(); ++i, ++j)
          {
            if(actualTProps[i] != expectedTProps[j])
              i--;

            if(j + 1 == expectedTProps.size())
            {
              if(actualTProps[i] != expectedTProps[j])
                out->error("Item_ID=%d property: expecting \"%s\" ({ %s}), got \"%s\" ({ %s})", derivationChain.first,
                           toString(expectedTProps[j]).c_str(), expected.c_str(), toString(actualTProps[i + 1]).c_str(), actual.c_str());
              else if(i + 1 != actualTProps.size())
                out->error("Item_ID=%d property: expecting { %s}, got { %s}", derivationChain.first, expected.c_str(), actual.c_str());
            }
          }
        }
      }
    },
    {
      "Section 7.3.9\n"
      "An identity derivation shall not be derived immediately from another identity\n"
      "derivation",
      [] (Box const& root, IReport* out)
      {
        auto graph = buildDerivationGraph(root);

        auto check = [&] (const std::list<uint32_t>& visited) {
            std::string last;

            for(auto v : visited)
            {
              if(graph.itemTypes[v] == "iden" && last == "iden")
                out->error("An identity derivation shall not be derived immediately from another identity (item_ID=%u)", v);

              last = graph.itemTypes[v];
            }
          };

        auto onError = [&] (const std::list<uint32_t>& visited) {
            out->error("Detected error in derivations: %s", graph.display(visited).c_str());
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

        auto onError = [&] (const std::list<uint32_t>& visited) {
            out->error("Detected error in derivations: %s", graph.display(visited).c_str());
          };

        auto check = [&] (const std::list<uint32_t>& visited) {
            std::vector<std::string> expected = { "iden", "grid", "iden", "iovl", "iden" };
            auto const numDerivations = (int)visited.size() - 1; // origin is a coded image: skip it

            if(numDerivations > (int)expected.size())
            {
              out->error("Too many derivations: %d (expected %d)", numDerivations, expected.size());
              return;
            }

            auto visitedIt = ++visited.begin(); // origin is a coded image: skip it
            bool error = false;

            for(int i = 0, expectIdx = 0; i < numDerivations && expectIdx < (int)expected.size(); ++i, ++visitedIt, ++expectIdx)
            {
              // iterate over optional transformations
              for(;;)
              {
                if(graph.itemTypes[*visitedIt] == expected[expectIdx])
                  break;

                expectIdx++;

                if(expectIdx == (int)expected.size())
                {
                  error = true;
                  break;
                }
              }

              if(error)
                break;
            }

            if(error)
              onError(visited);
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

