#include <cstring> // strcmp

#include "box_reader_impl.h"
#include "options.h"

extern const char *g_version;

const char *g_appName = "Compliance Warden";

std::vector<uint8_t> loadFile(const char *path);
std::vector<SpecDesc const *> &g_allSpecs();
void probeIsobmff(uint8_t *data, size_t size);
bool checkComplianceStd(Box const &file, SpecDesc const *spec);
bool checkComplianceJson(Box const &file, SpecDesc const *spec);
SpecDesc const *specFind(const char *name);
void printSpecDescription(const SpecDesc *spec);
void specListRules(const SpecDesc *spec);

namespace
{
void dump(Box const &box, int depth = 0)
{
  for(int i = 0; i < depth; ++i)
    printf("  ");

  printf("%s", toString(box.fourcc).c_str());
  printf("\n");

  for(auto &sym : box.syms) {
    if(!strcmp(sym.name, ""))
      continue;

    for(int i = 0; i < depth; ++i)
      printf("  ");

    printf("%s: %lld\n", sym.name, (long long)sym.value);
  }

  for(auto &child : box.children)
    dump(child, depth + 1);
}

bool specCheck(const SpecDesc *spec, const char *filename, uint8_t *data, size_t size, bool isJson)
{
  BoxReader topReader;
  topReader.br = { data, (int)size };
  topReader.myBox.original = data;
  topReader.myBox.position = 0;
  topReader.myBox.size = size;
  topReader.myBox.fourcc = FOURCC("root");
  topReader.specs = { spec };

  const std::string fnStr(filename);

  {
    // remove path
    auto fnPos = fnStr.find_last_of('/');
    auto fn = fnStr.substr(fnPos + 1);
    auto fnPtr = fn.c_str();

    while(*fnPtr) {
      topReader.myBox.syms.push_back({ "filename", *fnPtr, 8 });
      fnPtr++;
    }
  }

  auto const extPos = fnStr.find_last_of('.');

  if(
    extPos == std::string::npos ||
    (fnStr.substr(extPos) != ".obu" && fnStr.substr(extPos) != ".av1" && fnStr.substr(extPos) != ".av1b")) {
    probeIsobmff(data, size);

    auto parseFunc = getParseFunction(topReader.myBox.fourcc);
    parseFunc(&topReader);
  }

  if(0)
    dump(topReader.myBox);

  if(isJson)
    return checkComplianceJson(topReader.myBox, spec);
  else
    return checkComplianceStd(topReader.myBox, spec);
}

void fprintVersion(FILE *const stream)
{
  fprintf(stream, "%s, version %s\n", g_appName, g_version);
  fflush(stream);
}

void printUsageAndExit(const char *progName)
{
  fprintVersion(stderr);
  fprintf(stderr, "\nUsage:\n");
  fprintf(stderr, "- Run conformance:          %s <spec> input.mp4 [json]\n", progName);
  fprintf(stderr, "- List specifications:      %s list\n", progName);
  fprintf(stderr, "- List specification rules: %s <spec> list\n", progName);
  fprintf(stderr, "- Print version:            %s version\n", progName);
  exit(1);
}
}

#ifndef CW_WASM

// TODO: remove: legacy parsing introduced in July 2023 for v32
int mainLegacy(int argc, const char *argv[])
{
  if(argc < 2 || argc > 4)
    printUsageAndExit(argv[0]);

  if(!strcmp(argv[1], "list")) {
    for(auto &spec : g_allSpecs())
      printSpecDescription(spec);

    return 0;
  } else if(!strcmp(argv[1], "version")) {
    fprintVersion(stdout);
    return 0;
  } else if(argc < 3)
    printUsageAndExit(argv[0]);

  auto spec = specFind(argv[1]);

  if(!strcmp(argv[2], "list")) {
    specListRules(spec);
    return 0;
  }

  auto const outputJson = (argc < 4) ? false : !strcmp(argv[3], "json");

  auto buf = loadFile(argv[2]);

  return specCheck(spec, argv[2], buf.data(), (int)buf.size(), outputJson);
}

int main(int argc, const char *argv[])
{
  bool help = false, list = false, version = false, testMode = false;
  std::string specName, format = "text";

  OptionHandler opt;
  opt.add("s", "spec", &specName, "Specification name.");
  opt.add("f", "format", &format, "Output format: \"raw\" (default), or \"json\"");
  opt.addFlag("l", "list", &list, "List available specifications or available rules.");
  opt.addFlag("v", "version", &version, "Print version and exit.");
  opt.addFlag("h", "help", &help, "Print usage and exit.");
  // TODO: remove (used to support deprecated legacy mode)
  opt.addFlag("t", "test", &testMode, "Don't print warnings when switching to legacy mode.");

  auto urls = opt.parse(argc, argv);

  if(help) {
    fprintVersion(stdout);
    opt.printHelp(stdout);
    return 0;
  }

  if(version) {
    fprintVersion(stdout);
    return 0;
  }

  if(list) {
    if(specName.empty()) {
      for(auto &spec : g_allSpecs())
        printSpecDescription(spec);
    } else {
      auto spec = specFind(specName.c_str());
      specListRules(spec);
    }
    return 0;
  }

  if(format != "text" && format != "json") {
    fprintf(stderr, "invalid format, only \"text\" or \"json\" are supported");
    return 1;
  }

  if(specName.empty() || urls.size() != 1) {
    if(!testMode) {
      fprintf(stderr, "+------------------------------------------------------------+\n");
      fprintf(stderr, "| /!\\ Failed argument parsing. Switching to legacy mode. /!\\ |\n");
      fprintf(stderr, "+------------------------------------------------------------+\n");
      opt.printHelp(stderr);
    } else {
      for(int i = 1, j = 1; i < argc; ++i, ++j) {
        if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) {
          j--;
          continue;
        }
        argv[j] = argv[i];
      }
      argc--;
    }
    return mainLegacy(argc, argv);
    // fprintf(stderr, "expected one input file, got %" urls.size());
    // return 1;
  }

  auto spec = specFind(specName.c_str());
  auto buf = loadFile(urls[0].c_str());
  return specCheck(spec, urls[0].c_str(), buf.data(), (int)buf.size(), format == "json");
}

#else

/* ***** emscripten exports ***** */

extern "C" {
struct SpecDesc;
SpecDesc const *specFindC(const char *name);
void specListRulesC(const SpecDesc *spec);
void specCheckC(const SpecDesc *spec, const char *filename, uint8_t *data, size_t size);
void printVersion();
}

SpecDesc const *specFindC(const char *name)
{
  return specFind(name);
}

void specListRulesC(const SpecDesc *spec)
{
  specListRules(spec);
}

void specCheckC(const SpecDesc *spec, const char *filename, uint8_t *data, size_t size)
{
  specCheck(spec, filename, data, size, false);
}

void printVersion()
{
  fprintVersion(stdout);
}

#endif /*CW_WASM*/
