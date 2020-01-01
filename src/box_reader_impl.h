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
    BoxReader subReader;
    subReader.spec = spec;
    subReader.myBox.size = br.u(32);
    subReader.myBox.fourcc = br.u(32);
    unsigned boxHeaderSize = 8;

    if(subReader.myBox.size == 1)
    {
      subReader.myBox.size = br.u(64); // large size
      boxHeaderSize += 8;
    }

    ENSURE(subReader.myBox.size >= boxHeaderSize, "BoxReader::box(): box size %llu < %u bytes (fourcc='%s')",
           subReader.myBox.size, boxHeaderSize, toString(subReader.myBox.fourcc).c_str());

    subReader.br = br.sub(int(subReader.myBox.size - boxHeaderSize));
    auto pos = subReader.br.m_pos;
    auto parseFunc = selectBoxParseFunction(subReader.myBox.fourcc);
    parseFunc(&subReader);
    myBox.children.push_back(std::move(subReader.myBox));

    ENSURE((uint64_t)subReader.br.m_pos == pos + (subReader.myBox.size - boxHeaderSize) * 8,
           "Box '%s': read %d bits instead of %llu bits",
           toString(subReader.myBox.fourcc).c_str(), subReader.br.m_pos - pos, (subReader.myBox.size - boxHeaderSize) * 8);
  }

  BitReader br;

  Box myBox;
  const SpecDesc* spec = nullptr;

private:
  ParseBoxFunc* selectBoxParseFunction(uint32_t fourcc)
  {
    // try first custom parse function
    if(spec && spec->getParseFunction)
      if(auto func = spec->getParseFunction(fourcc))
        return func;

    return getParseFunction(fourcc);
  }
};

