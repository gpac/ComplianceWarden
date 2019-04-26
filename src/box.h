#pragma once

#include <cstdint>
#include <vector>

struct Symbol
{
  const char* name;
  int64_t value;
};

struct Box
{
  uint32_t fourcc;
  std::vector<Box> children;
  std::vector<Symbol> syms;

  void add(const char* name, int64_t value)
  {
    syms.push_back({ name, value });
  }
};

template<size_t N>
constexpr uint32_t FOURCC(const char(&tab)[N])
{
  static_assert(N == 4 + 1);
  uint32_t r = 0;

  for(int i = 0; i < 4; ++i)
  {
    r <<= 8;
    r |= tab[i];
  }

  return r;
}

