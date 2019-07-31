#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

#include "box.h"
#include "common_boxes.h"
#include "reader.h"
#include "spec.h"

using namespace std;

void ENSURE(bool cond, const char* format, ...)
{
  if(!cond)
  {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(args);
    exit(1);
  }
}

std::string toString(uint32_t fourcc)
{
  char fourccStr[5] = {};
  snprintf(fourccStr, 5, "%c%c%c%c",
           (fourcc >> 24) & 0xff,
           (fourcc >> 16) & 0xff,
           (fourcc >> 8) & 0xff,
           (fourcc >> 0) & 0xff);
  return fourccStr;
}

vector<uint8_t> loadFile(const char* path)
{
  FILE* fp = fopen(path, "rb");
  ENSURE(fp, "Can't open '%s' for reading", path);

  vector<uint8_t> buf(100 * 1024 * 1024);
  auto const size = fread(buf.data(), 1, buf.size(), fp);
  fclose(fp);
  buf.resize(size);
  buf.shrink_to_fit();
  return buf;
}

void dump(Box const& box, int depth = 0)
{
  for(int i = 0; i < depth; ++i)
    printf("  ");

  printf("%s", toString(box.fourcc).c_str());
  printf("\n");

  for(auto& sym : box.syms)
  {
    if(!strcmp(sym.name, ""))
      continue;

    for(int i = 0; i < depth; ++i)
      printf("  ");

    printf("%s: %lld\n", sym.name, (long long)sym.value);
  }

  for(auto& child : box.children)
    dump(child, depth + 1);
}

void checkCompliance(Box const& file, SpecDesc const* spec)
{
  struct Report : IReport
  {
    void error(const char* fmt, ...) override
    {
      va_list args;
      va_start(args, fmt);
      printf("[Rule #%d] ", ruleIdx);
      vprintf(fmt, args);
      va_end(args);
      printf("\n");
      ++errorCount;
    }

    int ruleIdx = 0;
    int errorCount = 0;
  };

  Report out;

  for(auto& rule : spec->rules)
  {
    rule.check(file, &out);
    out.ruleIdx++;
  }

  if(out.errorCount)
    fprintf(stdout, "%d error(s).\n", out.errorCount);
  else
    fprintf(stdout, "No errors.\n");
}

std::vector<SpecDesc const*>& g_allSpecs()
{
  static std::vector<SpecDesc const*> allSpecs;
  return allSpecs;
}

int registerSpec(SpecDesc const* spec)
{
  g_allSpecs().push_back(spec);
  return 0;
}

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

struct BoxReader : IReader
{
  bool empty() override
  {
    return br.empty();
  }

  int64_t sym(const char* name, int bits) override
  {
    auto val = br.u(bits);
    myBox.syms.push_back({ name, val });
    return val;
  }

  void box() override
  {
    BoxReader subReader;
    subReader.spec = spec;
    subReader.myBox.size = br.u(32);
    subReader.myBox.fourcc = br.u(32);

    if(subReader.myBox.size == 1)
      subReader.myBox.size = br.u(64); // large size

    ENSURE(subReader.myBox.size >= 8, "BoxReader::box(): box size %d < 8 bytes (fourcc='%s')",
           subReader.myBox.size, toString(subReader.myBox.fourcc).c_str());

    subReader.br = br.sub(int(subReader.myBox.size - 8));
    auto pos = subReader.br.m_pos;
    auto parseFunc = selectBoxParseFunction(subReader.myBox.fourcc);
    parseFunc(&subReader);
    myBox.children.push_back(std::move(subReader.myBox));

    ENSURE((uint64_t)subReader.br.m_pos == pos + (subReader.myBox.size - 8) * 8,
           "Box '%s': read %d bits instead of %llu bits",
           toString(subReader.myBox.fourcc).c_str(), subReader.br.m_pos - pos, (subReader.myBox.size - 8) * 8);
  }

  BitReader br;

  Box myBox;
  const SpecDesc* spec;

private:
  ParseBoxFunc* selectBoxParseFunction(uint32_t fourcc)
  {
    // try first custom parse function
    if(spec->getParseFunction)
      if(auto func = spec->getParseFunction(fourcc))
        return func;

    return getParseFunction(fourcc);
  }
};

SpecDesc const* specFind(const char* name)
{
  for(auto& spec : g_allSpecs())
    if(strcmp(spec->name, name) == 0)
      return spec;

  fprintf(stderr, "Spec '%s' not found\n", name);
  exit(1);
}

void specListRules(const SpecDesc* spec)
{
  fprintf(stdout, "Specification name: %s\n", spec->name);
  fprintf(stdout, "            detail: %s\n\n", spec->caption);
  int ruleIdx = 0;

  for(auto& r : spec->rules)
  {
    fprintf(stdout, "Rule #%04d: %s\n\n", ruleIdx, r.caption);
    ruleIdx++;
  }
}

void specCheck(const SpecDesc* spec, uint8_t* data, size_t size)
{
  BoxReader topReader;
  topReader.br = { data, (int)size };
  topReader.myBox.size = size;
  topReader.myBox.fourcc = FOURCC("root");
  topReader.spec = spec;
  auto parseFunc = getParseFunction(topReader.myBox.fourcc);
  parseFunc(&topReader);

  if(0)
    dump(topReader.myBox);

  checkCompliance(topReader.myBox, spec);
}

int main(int argc, const char* argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s <spec> <list|input.mp4>\n", argv[0]);
    return 1;
  }

  auto spec = specFind(argv[1]);

  if(!strcmp(argv[2], "list"))
  {
    specListRules(spec);
  }
  else
  {
    auto buf = loadFile(argv[2]);
    specCheck(spec, buf.data(), (int)buf.size());
  }

  return 0;
}

/*****************************************************************************/

/*emscripten exports */
extern "C" {
struct SpecDesc;
SpecDesc const* specFindC(const char* name);
void specListRulesC(const SpecDesc* spec);
void specCheckC(const SpecDesc* spec, uint8_t* data, size_t size);
}

SpecDesc const* specFindC(const char* name)
{
  return specFind(name);
}

void specListRulesC(const SpecDesc* spec)
{
  specListRules(spec);
}

void specCheckC(const SpecDesc* spec, uint8_t* data, size_t size)
{
  specCheck(spec, data, size);
}

