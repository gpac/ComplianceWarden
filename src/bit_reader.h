#pragma once

#include <cstdint>

void ENSURE(bool cond, const char* format, ...);

struct BitReader
{
  uint8_t* src;
  int size = 0;

  int64_t u(int n)
  {
    uint64_t r = 0;

    for(int i = 0; i < n; ++i)
      r = (r << 1) | bit();

    return r;
  }

  BitReader sub(int byteCount)
  {
    ENSURE((m_pos % 8) == 0, "BitReader::sub(): not byte-aligned");
    ENSURE(byteCount <= size, "BitReader::sub(): overflow asking %d bytes with %d available", byteCount, size - m_pos);

    auto sub = BitReader { src + m_pos / 8, byteCount };
    m_pos += byteCount * 8;
    return sub;
  }

  int bit()
  {
    const int byteOffset = m_pos / 8;
    const int bitOffset = m_pos % 8;

    ENSURE(byteOffset < size, "BitReader::bit() overflow");

    m_pos++;
    return (src[byteOffset] >> (7 - bitOffset)) & 1;
  }

  bool empty() const
  {
    return m_pos / 8 >= size;
  }

  int m_pos = 0;
};

