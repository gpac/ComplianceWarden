#pragma once

#include <cstdint>
#include <vector>

struct Symbol
{
  const char* name;
  int64_t value;
  int numBits;
};

struct Box
{
  uint32_t fourcc;
  uint64_t size;
  std::vector<Box> children;
  std::vector<Symbol> syms;
};

