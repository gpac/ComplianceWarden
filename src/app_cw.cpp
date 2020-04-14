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
      fflush(stdout);
      ++errorCount;
    }

    void warning(const char*, ...) override
    {
      /*ignored*/
    }

    int ruleIdx = 0;
    int errorCount = 0;
  };

  Report out;
  std::vector<int> ruleIdxError;

  auto printErrorRules = [&] () {
      fprintf(stdout, "\nSpecification description: %s\n", spec->caption);
      fprintf(stdout, "\nError rules description:\n");

      int ruleIdx = 0, errorIdx = 0;

      for(auto& rule : spec->rules)
      {
        while(ruleIdx > ruleIdxError[errorIdx])
        {
          errorIdx++;

          if(errorIdx == (int)ruleIdxError.size())
            return;
        }

        if(ruleIdxError[errorIdx] == ruleIdx)
          fprintf(stdout, "\n[Rule #%d] %s\n", ruleIdx, rule.caption);

        ruleIdx++;
      }
    };

  for(auto& rule : spec->rules)
  {
    auto curErrorCount = out.errorCount;

    rule.check(file, &out);

    if(curErrorCount != out.errorCount)
      ruleIdxError.push_back(out.ruleIdx);

    out.ruleIdx++;
  }

  if(!ruleIdxError.empty())
  {
    fprintf(stdout, "%d error(s).\n", out.errorCount);
    printErrorRules();
  }
  else
  {
    fprintf(stdout, "No errors.\n");
  }

  fflush(stdout);
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
  fflush(stderr);
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

  fflush(stdout);
}

void probeIsobmff(uint8_t* data, size_t size)
{
  ENSURE(size >= 8, "ISOBMFF probing: not enough bytes (%d bytes available). Aborting.", (int)size);

  uint32_t boxSize = 0;

  for(auto i = 0; i < 4; ++i)
    boxSize = (boxSize << 8) + data[i];

  ENSURE(boxSize >= 8, "ISOBMFF probing: first box size too small (%d bytes). Aborting.", (int)boxSize);
  ENSURE(boxSize <= size, "ISOBMFF probing: first box size too big (%d bytes when file size is %d bytes). Aborting.", (int)boxSize, size);

  for(auto i = 4; i < 7; ++i)
    ENSURE(isalpha(data[i]) || isdigit(data[i]) || isspace(data[i]), "Box type is neither an alphanumerics nor a space (box[%d]=\"%c\" (%d)). Aborting.", i, (char)data[i], (int)data[i]);
}

void specCheck(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size)
{
  probeIsobmff(data, size);

  BoxReader topReader;
  topReader.br = { data, (int)size };
  topReader.myBox.size = size;
  topReader.myBox.fourcc = FOURCC("root");
  topReader.spec = spec;

  {
    // remove path
    auto fnPos = std::string(filename).find_last_of('/');
    auto fn = std::string(filename).substr(fnPos + 1);
    auto fnPtr = fn.c_str();

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

