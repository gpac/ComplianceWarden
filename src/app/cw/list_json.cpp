#include "core/spec.h"

#include "json.h"

SpecDesc const *specFind(const char *name);

namespace
{
std::unique_ptr<Json::Object> printDesc(const SpecDesc *spec)
{
  auto o = std::make_unique<Json::Object>();
  o->content.push_back(std::make_unique<Json::Data>("specification_name", spec->name));
  o->content.push_back(std::make_unique<Json::Data>("detail", spec->caption));

  auto dependencies = std::make_unique<Json::Array>("dependencies");
  for(auto d : spec->dependencies)
    dependencies->content.push_back(std::make_unique<Json::Array::String>(d));

  return o;
}
}

void printSpecDescriptionJson(std::vector<SpecDesc const *> &specs)
{
  Json::Object root;
  for(auto spec : specs)
    root.content.push_back(printDesc(spec));
  root.serialize(0);
}

void specListRulesJson(const SpecDesc *spec)
{
  auto root = printDesc(spec);
  auto rules = std::make_unique<Json::Array>("rules");
  int ruleIdx = 0;
  for(auto &r : spec->rules) {
    auto o = std::make_unique<Json::Object>();
    o->content.push_back(std::make_unique<Json::Data>("rule_index", std::to_string(ruleIdx)));
    o->content.push_back(std::make_unique<Json::Data>("description", r.print()));
    rules->content.push_back(std::move(o));
    ruleIdx++;
  }
  root->content.push_back(std::move(rules));
  root->serialize(0);
}
