#include "core/spec.h"

#include <cstdarg>
#include <cstring> // strcmp
#include <sstream>
#include <stdexcept>

void ENSURE(bool cond, const char *format, ...);

std::vector<SpecDesc const *> &g_allSpecs()
{
  static std::vector<SpecDesc const *> allSpecs;
  return allSpecs;
}

int registerSpec(SpecDesc const *spec)
{
  g_allSpecs().push_back(spec);
  return 0;
}

SpecDesc const *specFind(const char *name)
{
  for(auto &spec : g_allSpecs())
    if(strcmp(spec->name, name) == 0)
      return spec;

  fprintf(stderr, "Spec '%s' not found: possible values are:", name);

  for(auto &spec : g_allSpecs())
    fprintf(stderr, " '%s'", spec->name);

  fprintf(stderr, ".\n");
  fflush(stderr);
  exit(1);
}

bool checkRuleSection(const SpecDesc &spec, const char *section, Box const &root)
{
  if(spec.valid && !spec.valid(root))
    return true;

  for(auto &rule : spec.rules) {
    std::stringstream ss(rule.print());
    std::string line;
    std::getline(ss, line);
    std::stringstream ssl(line);
    std::string word;
    ssl >> word;

    // optional id: go to next line
    if(word == "id:") {
      std::getline(ss, line);
      ssl = std::stringstream(line);
      ssl >> word;
    }

    if(word != "Section")
      throw std::runtime_error("Rule caption is misformed.");

    ssl >> word;

    if(word.rfind(section, 0) == 0) {
      struct Report : IReport {
        void error(const char *, ...) override { ++errorCount; }

        void warning(const char *, ...) override
        { /*ignored*/
        }

        void covered() override
        { /*ignored*/
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

std::vector<RuleDesc> concatRules(const std::initializer_list<const std::initializer_list<RuleDesc>> &rules)
{
  std::vector<RuleDesc> v;

  for(auto &r : rules)
    v.insert(v.end(), r.begin(), r.end());

  return v;
}
