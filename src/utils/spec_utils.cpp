#include "spec.h"
#include <cstdarg>
#include <cstring> // strcmp
#include <sstream>
#include <stdexcept>

void ENSURE(bool cond, const char* format, ...);

std::vector<SpecDesc const*> & g_allSpecs()
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

bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root)
{
  for(auto& rule : spec.rules)
  {
    std::stringstream ss(rule.caption);
    std::string line;
    std::getline(ss, line);
    std::stringstream ssl(line);
    std::string word;
    ssl >> word;

    if(word != "Section")
      throw std::runtime_error("Rule caption is misformed.");

    ssl >> word;

    if(word.rfind(section, 0) == 0)
    {
      struct Report : IReport
      {
        void error(const char*, ...) override
        {
          ++errorCount;
        }

        void warning(const char*, ...) override
        {
          /*ignored*/
        }

        int errorCount = 0;
      };
      Report r;
      rule.check(root, &r);

      if(r.errorCount)
        return false;
    }
  }

  return true;
}

std::vector<RuleDesc> concatRules(const std::initializer_list<const std::initializer_list<RuleDesc>>& rules)
{
  std::vector<RuleDesc> v;

  for(auto& r : rules)
    v.insert(v.end(), r.begin(), r.end());

  return v;
}

