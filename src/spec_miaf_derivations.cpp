#include "spec_utils_derivations.h"
#include "spec.h"
#include <algorithm>

bool isVisualSampleEntry(uint32_t fourcc);

const std::initializer_list<RuleDesc> getRulesMiafDerivations()
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
    {
      "Section 7.3.11.1\n"
      "All derivation chains shall originate from one or more coded images\n",
      [] (Box const& root, IReport* out)
      {
        auto graph = buildDerivationGraph(root);

        std::list<uint32_t> sourceItemIds;

        for(auto& c : graph.connections)
          sourceItemIds.push_back(c.src);

        for(auto& c : graph.connections)
        {
          auto src = std::find(sourceItemIds.begin(), sourceItemIds.end(), c.dst);

          if(src != sourceItemIds.end())
            sourceItemIds.erase(src);
        }

        for(auto srcItemId : sourceItemIds)
        {
          auto type = graph.itemTypes[srcItemId].c_str();
          uint32_t fourcc = type[3] + (type[2] << 8) + (type[1] << 16) + (type[0] << 24);

          if(!isVisualSampleEntry(fourcc))
            out->error("All derivation chains shall originate from one or more coded images: itemID=%u doesn't (type=%s).", srcItemId, type);
        }
      }
    },
  };
  return rulesDerivations;
}

