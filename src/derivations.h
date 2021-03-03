#pragma once

#include "box.h"
#include "fourcc.h"
#include <functional>
#include <list>
#include <map>
#include <string>

struct DerivationGraph
{
  // returns false on cyclic
  bool visit(uint32_t itemIdSrc, std::list<uint32_t> visited, std::function<void(const std::list<uint32_t> &)> onError, std::function<void(const std::list<uint32_t> &)> onTerminal);
  std::string display(const std::list<uint32_t>& visited);

  struct Connection
  {
    uint32_t src, dst;
  };

  std::vector<Connection> connections;
  std::map<uint32_t /*item_ID*/, std::string /*item_type*/> itemTypes;
};

DerivationGraph buildDerivationGraph(Box const& root);

