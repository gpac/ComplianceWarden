// This is how the app sees concrete specifications.
#pragma once

#include <vector>
#include "box.h"
#include "reader.h" // ParseBoxFunc

struct IReport
{
  // add a human-readable error to the report.
  virtual void error(const char* fmt, ...) = 0;
};

struct RuleDesc
{
  // human-readable description of the rule
  const char* caption;

  // apply this rule to the file 'root',
  // will push the results (messages) to the 'out' report.
  void (* check)(Box const& root, IReport* out);
};

struct SpecDesc
{
  // short name for the spec (used for command-line spec selection).
  const char* name;

  // human-readable description of the spec (name, version, date, etc.).
  const char* caption;

  // list of compliance checks for this spec.
  std::vector<RuleDesc> rules;

  ParseBoxFunc* (* getParseFunction)(uint32_t fourcc);
};

int registerSpec(SpecDesc const* spec);

