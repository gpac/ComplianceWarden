#pragma once

#include <cstdint>

struct IReader {
  virtual bool empty() = 0;
  virtual int64_t &sym(const char *name, int bits) = 0;
  virtual void box() = 0;
};

using ParseBoxFunc = void(IReader *br);
