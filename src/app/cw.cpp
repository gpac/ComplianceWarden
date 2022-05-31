#include "box_reader_impl.h"
#include <cstdarg>
#include <cstring> // strlen

extern const char* g_version;

const char* g_appName = "Compliance Warden";

std::vector<uint8_t> loadFile(const char* path);
std::vector<SpecDesc const*> & g_allSpecs();
void probeIsobmff(uint8_t* data, size_t size);
SpecDesc const* specFind(const char* name);
void printSpecDescription(const SpecDesc* spec);
void specListRules(const SpecDesc* spec);

namespace
{
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

int checkCompliance(Box const& file, SpecDesc const* spec)
{
  struct Report : IReport
  {
    void error(const char* fmt, ...) override
    {
      va_list args;
      va_start(args, fmt);
      printf("[%s][Rule #%d] Error: ", specName, ruleIdx);
      vprintf(fmt, args);
      va_end(args);
      printf("\n");
      fflush(stdout);
      ++errorCount;
    }

    void warning(const char* fmt, ...) override
    {
      va_list args;
      va_start(args, fmt);
      printf("[%s][Rule #%d] Warning: ", specName, ruleIdx);
      vprintf(fmt, args);
      va_end(args);
      printf("\n");
      fflush(stdout);
      ++warningCount;
    }

    int ruleIdx = 0;
    int errorCount = 0;
    int warningCount = 0;
    const char* specName = nullptr;
  };

  Report out;
  out.specName = spec->name;
  std::vector<int> ruleIdxEvent;

  auto printErrorRules = [&] () {
    fprintf(stdout, "\n===== Involved rules descriptions:\n");

    int ruleIdx = 0, eventIdx = 0;

    for(auto& rule : spec->rules)
    {
      while(ruleIdx > ruleIdxEvent[eventIdx])
      {
        eventIdx++;

        if(eventIdx == (int)ruleIdxEvent.size())
          return;
      }

      if(ruleIdxEvent[eventIdx] == ruleIdx)
        fprintf(stdout, "\n[%s][Rule #%d] %s\n", spec->name, ruleIdx, rule.caption);

      ruleIdx++;
    }
  };

  auto const cols = 40;
  {
    fprintf(stdout, "+%s+\n", std::string(cols - 2, '-').c_str());
    auto const spacing = cols - 11 - 2 - strlen(spec->name);
    fprintf(stdout, "|%s%s validation%s|\n", std::string(spacing / 2, ' ').c_str(), spec->name, std::string((spacing + 1) / 2, ' ').c_str());
    fprintf(stdout, "+%s+\n\n", std::string(cols - 2, '-').c_str());
  }

  fprintf(stdout, "Specification description: %s\n\n", spec->caption);

  for(auto& rule : spec->rules)
  {
    auto curEventCount = out.errorCount + out.warningCount;

    try
    {
      if(spec->valid(file))
        rule.check(file, &out);
    }
    catch(std::exception const& e)
    {
      out.error("ABORTED TEST: %s\n", e.what());
    }

    if(curEventCount != out.errorCount + out.warningCount)
      ruleIdxEvent.push_back(out.ruleIdx);

    out.ruleIdx++;
  }

  if(!ruleIdxEvent.empty())
  {
    fprintf(stdout, "\n%s\n", std::string(cols, '=').c_str());
    fprintf(stdout, "[%s] %d error(s), %d warning(s).\n", spec->name, out.errorCount, out.warningCount);
    fprintf(stdout, "%s\n", std::string(cols, '=').c_str());
    printErrorRules();
    fprintf(stdout, "\n");
  }
  else
  {
    fprintf(stdout, "%s\n", std::string(cols, '=').c_str());
    fprintf(stdout, "[%s] No errors.\n", spec->name);
    fprintf(stdout, "%s\n\n", std::string(cols, '=').c_str());
  }

  auto eventCount = out.errorCount + out.warningCount;

  for(auto dep : spec->dependencies)
    eventCount += checkCompliance(file, specFind(dep));

  fflush(stdout);

  return eventCount;
}

int specCheck(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size)
{
  BoxReader topReader;
  topReader.br = { data, (int)size };
  topReader.myBox.original = data;
  topReader.myBox.position = 0;
  topReader.myBox.size = size;
  topReader.myBox.fourcc = FOURCC("root");
  topReader.specs = { spec };

  const std::string fnStr(filename);

  auto const extPos = fnStr.find_last_of('.');

  if(extPos == std::string::npos ||
     (fnStr.substr(extPos) != ".obu" && fnStr.substr(extPos) != ".av1" && fnStr.substr(extPos) != ".av1b"))
  {
    probeIsobmff(data, size);

    {
      // remove path
      auto fnPos = fnStr.find_last_of('/');
      auto fn = fnStr.substr(fnPos + 1);
      auto fnPtr = fn.c_str();

      while(*fnPtr)
      {
        topReader.myBox.syms.push_back({ "filename", *fnPtr, 8 });
        fnPtr++;
      }
    }

    auto parseFunc = getParseFunction(topReader.myBox.fourcc);
    parseFunc(&topReader);
  }

  if(0)
    dump(topReader.myBox);

  return checkCompliance(topReader.myBox, spec);
}

void fprintVersion(FILE* const stream)
{
  fprintf(stream, "%s, version %s.\n", g_appName, g_version);
  fflush(stream);
}

void printUsageAndExit(const char* progName)
{
  fprintVersion(stderr);
  fprintf(stderr, "\nUsage:\n");
  fprintf(stderr, "- Run conformance:          %s <spec> input.mp4\n", progName);
  fprintf(stderr, "- List specifications:      %s list\n", progName);
  fprintf(stderr, "- List specification rules: %s <spec> list\n", progName);
  fprintf(stderr, "- Print version:            %s version\n", progName);
  exit(1);
}
}

#ifndef CW_WASM

int main(int argc, const char* argv[])
{
  if(argc < 2 || argc > 3)
    printUsageAndExit(argv[0]);

  if(!strcmp(argv[1], "list"))
  {
    for(auto& spec : g_allSpecs())
      printSpecDescription(spec);

    return 0;
  }
  else if(!strcmp(argv[1], "version"))
  {
    fprintVersion(stdout);
    return 0;
  }
  else if(argc < 3)
    printUsageAndExit(argv[0]);

  auto spec = specFind(argv[1]);

  if(!strcmp(argv[2], "list"))
  {
    specListRules(spec);
    return 0;
  }

  auto buf = loadFile(argv[2]);

  return specCheck(spec, argv[2], buf.data(), (int)buf.size());
}

#else

/* ***** emscripten exports ***** */

extern "C" {
struct SpecDesc;
SpecDesc const* specFindC(const char* name);
void specListRulesC(const SpecDesc* spec);
void specCheckC(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size);
void printVersion();
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

void printVersion()
{
  fprintVersion(stdout);
}

#endif /*CW_WASM*/

