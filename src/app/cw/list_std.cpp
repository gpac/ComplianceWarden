#include "core/spec.h"

SpecDesc const *specFind(const char *name);

void printSpecDescriptionStd(const SpecDesc *spec)
{
  fprintf(stdout, "================================================================================\n");
  fprintf(stdout, "Specification name: %s\n", spec->name);
  fprintf(stdout, "            detail: %s\n", spec->caption);
  fprintf(stdout, "        depends on:");

  if(spec->dependencies.empty()) {
    fprintf(stdout, " none.\n");
  } else {
    for(auto d : spec->dependencies)
      fprintf(stdout, " \"%s\"", d);

    fprintf(stdout, " specifications.\n");
  }

  fprintf(stdout, "================================================================================\n\n");
}

void specListRulesStd(const SpecDesc *spec)
{
  fprintf(stdout, "////////////////////// Beginning of \"%s\" specification.\n\n", spec->name);
  printSpecDescriptionStd(spec);

  int ruleIdx = 0;

  for(auto &r : spec->rules) {
    fprintf(stdout, "[%s] Rule #%04d: %s\n\n", spec->name, ruleIdx, r.print().c_str());
    ruleIdx++;
  }

  fprintf(stdout, "///////////////////////// End of \"%s\" specification.\n\n", spec->name);

  for(auto dep : spec->dependencies)
    specListRulesStd(specFind(dep));

  fflush(stdout);
}
