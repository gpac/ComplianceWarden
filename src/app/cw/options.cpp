#include "options.h"

std::vector<std::string> OptionHandler::parse(int argc, const char *argv[])
{
  std::vector<std::string> remaining;

  ArgQueue args;
  for(int i = 1; i < argc; ++i) // skip argv[0]
    args.push(argv[i]);

  while(!args.empty()) {
    auto word = args.front();
    args.pop();

    if(word.substr(0, 1) != "-") {
      remaining.push_back(word);
      continue;
    }

    AbstractOption *opt = nullptr;

    for(auto &o : m_Options) {
      if(word == o->shortName || word == o->longName) {
        opt = o.get();
        break;
      }
    }

    if(!opt) {
      fprintf(stderr, "Unknown option: \"%s\"\n", word.c_str());
      exit(1);
    }

    opt->parse(args);
  }

  return remaining;
}

void OptionHandler::printHelp(FILE *f)
{
  fprintf(f, "\nUsage:\n");
  for(auto &o : m_Options) {
    auto s = o->shortName + ", " + o->longName;
    while(s.size() < 40)
      s += " ";
    fprintf(f, "    %s%s\n", s.c_str(), o->desc.c_str());
  }
  fprintf(f, "\n");
}
