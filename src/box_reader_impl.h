#pragma once

#include "bit_reader.h"
#include "spec.h"
#include "common_boxes.h"
#include "fourcc.h"

void ENSURE(bool cond, const char* format, ...);

struct BoxReader : IReader
{
  bool empty() override
  {
    return br.empty();
  }

  int64_t sym(const char* name, int bits) override
  {
    auto val = br.u(bits);
    myBox.syms.push_back({ name, val, bits });
    return val;
  }

  void box() override
  {
    uint64_t size;

    BoxReader subReader;
    subReader.specs = specs;
    subReader.myBox.original = myBox.original + br.m_pos / 8;
    subReader.myBox.position = myBox.position + br.m_pos / 8;
    size = subReader.myBox.size = br.u(32);
    subReader.myBox.syms.push_back({ "size", (int64_t)size, 32 });
    subReader.myBox.fourcc = br.u(32);
    subReader.myBox.syms.push_back({ "fourcc", (int64_t)subReader.myBox.fourcc, 32 });
    unsigned boxHeaderSize = 8;

    if(size == 1)
    {
      size = subReader.myBox.size = br.u(64); // large size
      subReader.myBox.syms.push_back({ "size", (int64_t)subReader.myBox.size, 64 });
      boxHeaderSize += 8;
    }
    else if(size == 0)
    {
      size = myBox.size + boxHeaderSize - br.m_pos / 8; // until the end of container - may be impossible to get right in some wrong cases
    }

    ENSURE(size >= boxHeaderSize, "BoxReader::box(): box size %llu < %u bytes (fourcc='%s')",
           size, boxHeaderSize, toString(subReader.myBox.fourcc).c_str());

    subReader.br = br.sub(int(size - boxHeaderSize));
    auto pos = subReader.br.m_pos;
    auto parseFunc = selectBoxParseFunction(subReader.myBox.fourcc);
    parseFunc(&subReader);
    myBox.children.push_back(std::move(subReader.myBox));

    ENSURE((uint64_t)subReader.br.m_pos == pos + (size - boxHeaderSize) * 8,
           "Box '%s': read %d bits instead of %llu bits",
           toString(subReader.myBox.fourcc).c_str(), subReader.br.m_pos - pos, (size - boxHeaderSize) * 8);
  }

  BitReader br;

  Box myBox {};
  std::vector<const SpecDesc*> specs;

private:
  ParseBoxFunc* selectBoxParseFunction(uint32_t fourcc)
  {
    // try first custom parse function
    for(auto spec : specs)
      if(spec->getParseFunction)
        if(auto func = spec->getParseFunction(fourcc))
          return func;

    return getParseFunction(fourcc);
  }
};

