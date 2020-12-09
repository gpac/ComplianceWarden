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

  uint8_t* original;
  uint64_t position;

  std::vector<Box> children;
  std::vector<Symbol> syms;
};

