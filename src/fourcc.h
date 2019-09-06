#pragma once

#include <cstdint>
#include <string>

template<size_t N>
constexpr uint32_t FOURCC(const char(&tab)[N])
{
  static_assert(N == 4 + 1, "FOURCC input size must be 4 + 1");
  uint32_t r = 0;

  for(int i = 0; i < 4; ++i)
  {
    r <<= 8;
    r |= tab[i];
  }

  return r;
}

std::string toString(uint32_t fourcc);

