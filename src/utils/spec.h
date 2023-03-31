// This is how the app sees concrete specifications.
#pragma once

#include "box.h"
#include "box_reader.h" // ParseBoxFunc
#include <string>
#include <vector>

struct IReport
{
  // add a human-readable error or warning to the report.
  virtual void error(const char* fmt, ...) = 0;
  virtual void warning(const char* fmt, ...) = 0;

  // advertize that a test was exercized
  virtual void covered() = 0;
};

struct RuleDesc
{
  RuleDesc(const char* caption, const char* id, void(*check)(Box const &, IReport*)) : caption(caption), id(id), check(check) {}

  // partial constructor for when rule has no id
  RuleDesc(const char* caption, void(*check)(Box const &, IReport*)) : caption(caption), check(check) {}

  // human readable print
  std::string print() const
  {
    if(!id)
      return caption;

    return std::string("id: ") + id + "\n" + caption;
  }

  // human-readable description of the rule
  const char* caption;

  // optional id from the specification
  const char* id = nullptr;

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

  // list of specs which this spec depends on.
  std::vector<const char*> dependencies;

  // list of compliance checks for this spec.
  std::vector<RuleDesc> rules;

  // checks will only be executed if this returns true.
  bool (* valid)(Box const& root) = nullptr;
};

bool isIsobmff(Box const& root);

int registerSpec(SpecDesc const* spec);

