#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Json
{
void insertSpace(int indent);
std::string escape(const std::string &s);

struct ISerialize {
  virtual ~ISerialize() = default;
  virtual void serialize(int indent) const = 0;
};

struct Data : ISerialize {
  Data(std::string name, std::string value)
      : name(name)
      , value(value)
  {
  }

  std::string const name, value;
  void serialize(int indent) const final
  {
    insertSpace(indent);
    std::cout << "\"" << escape(name) << "\": \"" << escape(value) << "\"";
  }

  bool operator!=(const Data &other) const { return !(this->name == other.name && this->value == other.value); }
};

struct Object : ISerialize {
  std::vector<std::unique_ptr<ISerialize>> content; // either Data, Object or Array
  void serialize(int indent) const final
  {
    insertSpace(indent);
    std::cout << "{" << std::endl;

    for(auto &c : content) {
      c->serialize(indent + 1);

      if(c != content.back())
        std::cout << ",";

      std::cout << std::endl;
    }

    insertSpace(indent);
    std::cout << "}";
  }
};

struct Array : ISerialize {
  struct Int : ISerialize {
    int val;
    Int(int val)
        : val(val)
    {
    }

    void serialize(int indent) const final
    {
      insertSpace(indent);
      std::cout << val;
    }
  };
  struct String : ISerialize {
    std::string val;
    String(std::string val)
        : val(val)
    {
    }

    void serialize(int indent) const final
    {
      insertSpace(indent);
      std::cout << "\"" << escape(val) << "\"";
    }
  };

  Array(std::string name)
      : name(name)
  {
  }

  const std::string name;
  std::vector<std::unique_ptr<ISerialize>> content;
  void serialize(int indent) const final
  {
    insertSpace(indent);
    std::cout << "\"" << escape(name) << "\": [" << std::endl;

    for(auto &c : content) {
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
