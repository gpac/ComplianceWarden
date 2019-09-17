#include "box_reader_impl.h"
#include "fourcc.h"
#include <cstdarg>
#include <cstring>

/* ***** utils ***** */

std::vector<uint8_t> loadFile(const char* path);

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

/* ***** specs ***** */

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

void specCheck(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size)
{
  BoxReader topReader;
  topReader.br = { data, (int)size };
  topReader.myBox.size = size;
  topReader.myBox.fourcc = FOURCC("root");
  topReader.spec = spec;

  {
    auto fnPtr = filename;

    while(*fnPtr)
    {
      topReader.myBox.syms.push_back({ "filename", *fnPtr, 8 });
      fnPtr++;
    }
  }

  auto parseFunc = getParseFunction(topReader.myBox.fourcc);
  parseFunc(&topReader);

  if(0)
    dump(topReader.myBox);

  checkCompliance(topReader.myBox, spec);
}

/* ***** main ***** */

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
    specCheck(spec, argv[2], buf.data(), (int)buf.size());
  }

  return 0;
}

/* ***** emscripten exports ***** */

extern "C" {
struct SpecDesc;
SpecDesc const* specFindC(const char* name);
void specListRulesC(const SpecDesc* spec);
void specCheckC(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size);
}

SpecDesc const* specFindC(const char* name)
{
  return specFind(name);
}

void specListRulesC(const SpecDesc* spec)
{
  specListRules(spec);
}

void specCheckC(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size)
{
  specCheck(spec, filename, data, size);
}

