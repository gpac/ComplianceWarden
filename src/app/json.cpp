#include "json.h"

#include <iomanip> // setw, setfill

namespace Json
{
void insertSpace(int indent)
{
  static auto const space = "  ";

  for(int i = 0; i < indent; ++i)
    std::cout << space;
}

std::string escape(const std::string &s)
{
  std::ostringstream o;

  for(auto c = s.cbegin(); c != s.cend(); ++c) {
    if(*c == '"' || *c == '\\' || ('\x00' <= *c && *c <= '\x1f'))
      o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
    else
      o << *c;
  }

  return o.str();
}
}
