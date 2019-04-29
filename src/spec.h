#pragma once

#include <vector>
#include "box.h"

struct IReport
{
  virtual void error(const char* fmt, ...) = 0;
};

struct RuleDesc
{
  const char* caption;
  void (* check)(Box const& root, IReport* out);
};

struct SpecDesc
{
  const char* name;
  const char* caption;
  std::vector<RuleDesc> rules;
};

int registerSpec(SpecDesc const* spec);

