#pragma once

#include "box.h"
#include <cassert>

struct BitReader
{
  const uint8_t* src;
  const int size = 0;

  int64_t u(int n)
  {
    uint64_t r = 0;

    for(int i = 0; i < n; ++i)
      r = (r << 1) | bit();

    return r;
  }

  int bit()
  {
    const int byteOffset = m_pos / 8;
    const int bitOffset = m_pos % 8;

    assert(byteOffset < size);

    m_pos++;
    return (src[byteOffset] >> (7 - bitOffset)) & 1;
  }

  bool empty() const
  {
    return m_pos / 8 >= size;
  }

  int m_pos = 0;
};

using ParseBoxFunc = void(BitReader & br, Box & box);

