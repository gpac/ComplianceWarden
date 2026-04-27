#pragma once

#include "core/box_reader.h"

#define READ_UNTIL_NEXT_BYTE(readBits)                                                                                 \
  if(readBits % 8) {                                                                                                   \
    auto remainderBits = 8 - (readBits % 8);                                                                           \
    br->sym("bits", remainderBits);                                                                                    \
    readBits += remainderBits;                                                                                         \
  }

struct ReaderBits : IReader {
  ReaderBits(IReader *delegate)
      : delegate(delegate)
  {
  }

  virtual ~ReaderBits() {}

  bool empty() { return delegate->empty(); }

  int64_t &sym(const char *name, int bits)
  {
    count += bits;
    return delegate->sym(name, bits);
  }

  void box() { delegate->box(); }

  IReader *delegate = nullptr;
  int64_t count = 0;
};

inline uint64_t leb128_read(IReader *br)
{
  uint64_t value = 0;

  for(int i = 0; i < 8; i++) {
    uint8_t leb128_byte = br->sym("leb128_byte", 8);
    value |= (((uint64_t)(leb128_byte & 0x7f)) << (i * 7));

    if(!(leb128_byte & 0x80))
      break;
  }

  return value;
}
