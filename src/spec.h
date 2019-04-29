#pragma once

#include <vector>
#include "box.h"

struct IOutput
{
  virtual void error(const char* fmt, ...) = 0;
};

struct RuleDesc
{
  const char* caption;
  void (* check)(Box const& root, IOutput* out);
};

struct SpecDesc
{
  std::vector<RuleDesc> rules;
};

