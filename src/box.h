#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>

struct Box
{
  uint32_t fourcc;
  std::vector<Box> children;

  void add(const char* name, int64_t value)
  {
    if(0)
      printf("%s: %d\n", name, (int)value);
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

