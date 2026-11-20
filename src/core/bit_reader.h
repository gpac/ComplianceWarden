#pragma once

#include <cstdint>

void ENSURE(bool cond, const char *format, ...);

struct BitReader {
  uint8_t *src;
  int64_t size = 0; // bytes

  int64_t u(int n)
  {
    uint64_t r = 0;

    for(int i = 0; i < n; ++i)
      r = (r << 1) | bit();

    return r;
  }

  BitReader sub(int64_t byteCount)
  {
    ENSURE((m_pos % 8) == 0, "BitReader::sub(): not byte-aligned");
    ENSURE(
      byteCount <= size, "BitReader::sub(): overflow asking %lld bytes with %lld available", (long long)byteCount,
      (long long)(size - m_pos));

    auto sub = BitReader{ src + m_pos / 8, byteCount };
    m_pos += byteCount * 8;
    return sub;
  }

  int bit()
  {
    const int64_t byteOffset = m_pos / 8;
    const int bitOffset = m_pos % 8;

    ENSURE(byteOffset < size, "BitReader::bit() overflow");

    m_pos++;
    return (src[byteOffset] >> (7 - bitOffset)) & 1;
  }

  bool empty() const { return m_pos / 8 >= size; }

  int64_t m_pos = 0; // bits
};
