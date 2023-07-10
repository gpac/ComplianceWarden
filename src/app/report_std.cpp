#include <cstdarg>
#include <cstdio>
#include <cstring> // strlen
#include <string>

#include "spec.h"

SpecDesc const *specFind(const char *name);

bool checkComplianceStd(Box const &file, SpecDesc const *spec)
{
  // early exit if pre-check fails: this spec doesn't apply
  if(spec->valid && !spec->valid(file))
    return 0;

  struct Report : IReport {
    void error(const char *fmt, ...) override
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

    void warning(const char *fmt, ...) override
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

    void covered() override {}

    int ruleIdx = 0;
    int errorCount = 0;
    int warningCount = 0;
    const char *specName = nullptr;
  };

  Report out;
  out.specName = spec->name;
  std::vector<int> ruleIdxEvent;

  auto printErrorRules = [&]() {
    fprintf(stdout, "\n===== Involved rules descriptions:\n");

    int ruleIdx = 0, eventIdx = 0;

    for(auto &rule : spec->rules) {
      while(ruleIdx > ruleIdxEvent[eventIdx]) {
        eventIdx++;

        if(eventIdx == (int)ruleIdxEvent.size())
          return;
      }

      if(ruleIdxEvent[eventIdx] == ruleIdx)
        fprintf(stdout, "\n[%s][Rule #%d] %s\n", spec->name, ruleIdx, rule.print().c_str());

      ruleIdx++;
    }
  };

  auto const cols = 40;
  {
    fprintf(stdout, "+%s+\n", std::string(cols - 2, '-').c_str());
    auto const spacing = cols - 11 - 2 - strlen(spec->name);
    fprintf(
      stdout, "|%s%s validation%s|\n", std::string(spacing / 2, ' ').c_str(), spec->name,
      std::string((spacing + 1) / 2, ' ').c_str());
    fprintf(stdout, "+%s+\n\n", std::string(cols - 2, '-').c_str());
  }

  fprintf(stdout, "Specification description: %s\n\n", spec->caption);

  for(auto &rule : spec->rules) {
    auto curEventCount = out.errorCount + out.warningCount;

    try {
      rule.check(file, &out);
    } catch(std::exception const &e) {
      out.error("ABORTED TEST: %s\n", e.what());
    }

    if(curEventCount != out.errorCount + out.warningCount)
      ruleIdxEvent.push_back(out.ruleIdx);

    out.ruleIdx++;
  }

  if(!ruleIdxEvent.empty()) {
    fprintf(stdout, "\n%s\n", std::string(cols, '=').c_str());
    fprintf(stdout, "[%s] %d error(s), %d warning(s).\n", spec->name, out.errorCount, out.warningCount);
    fprintf(stdout, "%s\n", std::string(cols, '=').c_str());
    printErrorRules();
    fprintf(stdout, "\n");
  } else {
    fprintf(stdout, "%s\n", std::string(cols, '=').c_str());
    fprintf(stdout, "[%s] No errors.\n", spec->name);
    fprintf(stdout, "%s\n\n", std::string(cols, '=').c_str());
  }

  auto eventCount = out.errorCount + out.warningCount;

  for(auto dep : spec->dependencies)
    eventCount += checkComplianceStd(file, specFind(dep));

  fflush(stdout);

  return eventCount > 0;
}
