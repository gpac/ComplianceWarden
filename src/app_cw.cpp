#include "box_reader_impl.h"
#include "fourcc.h"
#include <cstdarg>
#include <cstring>
#include <stdexcept>

extern const char* g_version;

const char* g_appName = "Compliance Warden";

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

void printSpecDescription(const SpecDesc* spec)
{
  fprintf(stdout, "================================================================================\n");
  fprintf(stdout, "Specification name: %s\n", spec->name);
  fprintf(stdout, "            detail: %s\n", spec->caption);
  fprintf(stdout, "        depends on:");

  if(spec->dependencies.empty())
  {
    fprintf(stdout, " none.\n");
  }
  else
  {
    for(auto d : spec->dependencies)
      fprintf(stdout, " \"%s\"", d);

    fprintf(stdout, " specifications.\n");
  }

  fprintf(stdout, "================================================================================\n\n");
}

void specListRules(const SpecDesc* spec)
{
  fprintf(stdout, "////////////////////// Beginning of \"%s\" specification.\n\n", spec->name);
  printSpecDescription(spec);

  int ruleIdx = 0;

  for(auto& r : spec->rules)
  {
    fprintf(stdout, "[%s] Rule #%04d: %s\n\n", spec->name, ruleIdx, r.caption);
    ruleIdx++;
  }

  fprintf(stdout, "///////////////////////// End of \"%s\" specification.\n\n", spec->name);

  for(auto dep : spec->dependencies)
    specListRules(specFind(dep));

  fflush(stdout);
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

void probeIsobmff(uint8_t* data, size_t size)
{
  ENSURE(size >= 8, "ISOBMFF probing: not enough bytes (%d bytes available). Aborting.", (int)size);

  uint64_t boxSize = 0;

  for(auto i = 0; i < 4; ++i)
    boxSize = (boxSize << 8) + data[i];

  if(boxSize == 1)
  {
    boxSize = 0;

    for(auto i = 0; i < 4; ++i)
      boxSize = (boxSize << 8) + data[i];
  }

  ENSURE(boxSize == 0 || boxSize >= 8, "ISOBMFF probing: first box size too small (%d bytes). Aborting.", (int)boxSize);
  ENSURE(boxSize <= size, "ISOBMFF probing: first box size too big (%d bytes when file size is %d bytes). Aborting.", (int)boxSize, size);

  for(auto i = 4; i < 7; ++i)
    ENSURE(isalpha(data[i]) || isdigit(data[i]) || isspace(data[i]), "Box type is neither an alphanumerics nor a space (box[%d]=\"%c\" (%d)). Aborting.", i, (char)data[i], (int)data[i]);
}

int specCheck(const SpecDesc* spec, const char* filename, uint8_t* data, size_t size)
{
  probeIsobmff(data, size);

  BoxReader topReader;
  topReader.br = { data, (int)size };
  topReader.myBox.original = data;
  topReader.myBox.position = 0;
  topReader.myBox.size = size;
  topReader.myBox.fourcc = FOURCC("root");
  topReader.specs = { spec };

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

  return checkCompliance(topReader.myBox, spec);
}

void fprintVersion(FILE* const stream)
{
  fprintf(stream, "%s, version %s.\n", g_appName, g_version);
  fflush(stream);
}

/* ***** emscripten exports ***** */

#ifdef CW_WASM

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

/* ***** main ***** */

#ifndef CW_WASM

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

#endif /*!CW_WASM*/

