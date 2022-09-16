#include "spec.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring> // strcmp

extern const char* g_version;

SpecDesc const* specFind(const char* name);

namespace Json
{
void insertSpace(int indent)
{
  static auto const space = "  ";

  for(int i = 0; i < indent; ++i)
    std::cout << space;
}

struct ISerialize
{
  virtual ~ISerialize() = default;
  virtual void serialize(int indent) const = 0;
};

struct Data : ISerialize
{
  Data(std::string name, std::string value) : name(name), value(value) {}

  std::string const name, value;
  void serialize(int indent) const final
  {
    insertSpace(indent);
    std::cout << "\"" << name << "\": \"" << value << "\"";
  }

  bool operator != (const Data& other) const
  {
    return !(this->name == other.name && this->value == other.value);
  }
};

struct Object : ISerialize
{
  std::vector<std::unique_ptr<ISerialize>> content; // either Data, Object or Array
  void serialize(int indent) const final
  {
    insertSpace(indent);
    std::cout << "{" << std::endl;

    for(auto& c: content)
    {
      c->serialize(indent + 1);

      if(c != content.back())
        std::cout << ",";

      std::cout << std::endl;
    }

    insertSpace(indent);
    std::cout << "}";
  }
};

struct Array : ISerialize
{
  struct String : ISerialize
  {
    std::string val;
    String(std::string val) : val(val) {}

    void serialize(int indent) const final
    {
      insertSpace(indent);
      std::cout << "\"" << val << "\"";
    }
  };

  Array(std::string name) : name(name) {}

  const std::string name;
  std::vector<std::unique_ptr<ISerialize>> content;
  void serialize(int indent) const final
  {
    insertSpace(indent);
    std::cout << "\"" << name << "\": [" << std::endl;

    for(auto& c: content)
    {
      c->serialize(indent + 1);

      if(c != content.back())
        std::cout << ",";

      std::cout << std::endl;
    }

    insertSpace(indent);
    std::cout << "]";
  }
};
}

namespace
{
int checkComplianceJsonSpec(Box const& file, SpecDesc const* spec, Json::Array* const array, int indent /*Romain: not sure it is used*/)
{
  // early exit if pre-check fails: this spec doesn't apply
  if(spec->valid && !spec->valid(file))
    return 0;

  auto root = std::make_unique<Json::Object>();
  root->content.push_back(std::make_unique<Json::Data>("specification", spec->name));
  auto successArray = std::make_unique<Json::Array>("successful_checks");
  auto errorArray = std::make_unique<Json::Array>("errors");
  auto warningArray = std::make_unique<Json::Array>("warnings");

  struct Report : IReport
  {
    void error(const char* fmt, ...) override
    {
      char buf[4096];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buf, sizeof(buf) - 1, fmt, args);
      va_end(args);

      auto o = std::make_unique<Json::Object>();
      o->content.push_back(std::make_unique<Json::Data>("rule", std::to_string(ruleIdx)));
      o->content.push_back(std::make_unique<Json::Data>("details", spec->rules[ruleIdx].caption));
      o->content.push_back(std::make_unique<Json::Data>("description", std::string(buf)));
      warningArray->content.push_back(std::move(o));

      ++errorCount;
    }

    void warning(const char* fmt, ...) override
    {
      char buf[4096];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buf, sizeof(buf) - 1, fmt, args);
      va_end(args);

      auto o = std::make_unique<Json::Object>();
      o->content.push_back(std::make_unique<Json::Data>("rule", std::to_string(ruleIdx)));
      o->content.push_back(std::make_unique<Json::Data>("details", spec->rules[ruleIdx].caption));
      o->content.push_back(std::make_unique<Json::Data>("description", std::string(buf)));
      warningArray->content.push_back(std::move(o));

      ++warningCount;
    }

    int ruleIdx = 0;
    int errorCount = 0;
    int warningCount = 0;
    SpecDesc const* spec = nullptr;
    Json::Array* successArray = nullptr;
    Json::Array* errorArray = nullptr;
    Json::Array* warningArray = nullptr;
  };

  Report out;
  out.spec = spec;
  out.successArray = successArray.get();
  out.errorArray = errorArray.get();
  out.warningArray = warningArray.get();

  for(auto& rule : spec->rules)
  {
    try
    {
      rule.check(file, &out);
    }
    catch(std::exception const& e)
    {
      out.error("ABORTED TEST: %s\n", e.what());
    }

    out.ruleIdx++;
  }

  root->content.push_back(std::move(successArray));
  root->content.push_back(std::move(errorArray));
  root->content.push_back(std::move(warningArray));
  array->content.push_back(std::move(root));

  auto eventCount = out.errorCount + out.warningCount;

  for(auto dep : spec->dependencies)
    eventCount += checkComplianceJsonSpec(file, specFind(dep), array, indent);

  return eventCount;
}
}

int checkComplianceJson(Box const& file, SpecDesc const* spec)
{
  Json::Object root;
  root.content.push_back(std::make_unique<Json::Data>("cw_version", g_version));

  {
    std::string filename;

    for(auto& field : file.syms)
      if(!strcmp(field.name, "filename"))
        filename.push_back((char)field.value);

    root.content.push_back(std::make_unique<Json::Data>("input_file", filename));
  }

  root.content.push_back(std::make_unique<Json::Data>("specification", spec->name));
  root.content.push_back(std::make_unique<Json::Data>("spec_name", spec->caption));

  auto depsArray = std::make_unique<Json::Array>("dependencies");
  auto depsArrayPtr = depsArray.get();
  root.content.push_back(std::move(depsArray));

  auto validationArray = std::make_unique<Json::Array>("validation");
  auto validationArrayPtr = validationArray.get();
  root.content.push_back(std::move(validationArray));

  int eventCount = 0;

  for(auto& dep : spec->dependencies)
  {
    depsArrayPtr->content.push_back(std::make_unique<Json::Array::String>(dep));
    eventCount += checkComplianceJsonSpec(file, spec, validationArrayPtr, 0);
  }

  root.serialize(0);
  std::cout << std::endl;
  return eventCount;
}

