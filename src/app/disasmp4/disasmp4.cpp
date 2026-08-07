// Keep this file standalone.
//
// $ ./disasmp4 tests/aac.mp4 > yo.asm
// $ ./disasmp4 tests/av1.obu > yo.asm
//
// $ nasm -f bin yo.asm -o test.mp4
//

#include "core/bit_reader.h"
#include "core/box_reader_impl.h"
#include "utils/av1_utils.h"

#include <algorithm> // std::remove
#include <cassert>
#include <cstdarg>
#include <cstring>
#include <set>

std::vector<uint8_t> loadFile(const char *path);
std::vector<SpecDesc const *> &g_allSpecs();

void indent(int depth)
{
  printf("%*s", depth * 4, "");
}

void println(int depth, const char *format, ...)
{
  indent(depth);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
}

using DumpFunc = void(BitReader &br, int depth);

std::string allocLabel(std::string name)
{
  static std::set<std::string> labels;

  // strip unwanted characters
  name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

  std::string r = name;

  int k = 1;

  while(labels.find(r) != labels.end()) {
    ++k;

    char buffer[256]{ };
    snprintf(buffer, sizeof buffer, "%s%d", name.c_str(), k);
    r = buffer;
  }

  labels.insert(r);

  return r;
}

void dump(Box const &box, int depth = 0)
{
  char fourcc[5]{ };

  for(int i = 0; i < 4; ++i)
    fourcc[i] += char((box.fourcc >> (3 - i) * 8) & 0xff);

  std::string slabel = allocLabel(fourcc);
  auto label = slabel.c_str();

  println(depth, "%s_start:", label);
  if(!slabel.empty()) {
    println(depth + 1, "dd BE(%s_end - %s_start)", label, label);
    println(depth + 1, "dd \"%s\"", fourcc);
  }

  {
    size_t accumulatedBits = 0;
    __int128_t accumulatedVal = 0;
    std::vector<std::string> dbNames;

    for(auto &sym : box.syms) {
      if(!strcmp(sym.name, "size") || !strcmp(sym.name, "fourcc"))
        continue; // already written

      // if (box.fourcc != FOURCC("mdat"))
      {
        accumulatedBits += sym.numBits;
        if(accumulatedBits > sizeof(accumulatedVal) * 8)
          assert(0); // too many unaligned bits: need to write a bitstream writer

        accumulatedVal <<= sym.numBits;
        accumulatedVal += sym.value;
        dbNames.push_back(std::string(sym.name) + "(" + std::to_string(sym.numBits) + ")");

        if((accumulatedBits % 8) == 0) {
          std::string s;

          if(sym.numBits > 0)
            s += "db ";

          bool isText = sym.numBits > 0;

          auto processNext8bits = [&]() {
            auto const val = ((uint8_t *)(&accumulatedVal))[accumulatedBits / 8 - 1];
            char hex[5] = { };
            snprintf(hex, 5, "0x%02X", val);
            s += hex;

            if(val < 32 || val > 126)
              isText = false;
          };

          while(accumulatedBits > 8) {
            processNext8bits();
            accumulatedBits -= 8;
            s += ", ";
          }

          if(sym.numBits > 0)
            processNext8bits();

          s += " ; ";

          for(auto &n : dbNames) {
            s += n + " ";

            if(isText) {
              s += "('";

              for(int i = 7; i >= 0; --i)
                if(((char *)&accumulatedVal)[i])
                  s += ((char *)&accumulatedVal)[i];

              s += "') ";
            }
          }

          println(depth + 1, "%s", s.c_str());

          accumulatedBits = 0;
          accumulatedVal = 0;
          dbNames = { };
        }
      }
    }

    assert(!accumulatedBits && !accumulatedVal);
  }

  for(auto &child : box.children)
    dump(child, depth + 1);

  println(depth, "%s_end:", label);
}

int main(int argc, const char *argv[])
{
  if(argc != 2) {
    fprintf(stderr, "Usage: %s input.mp4\n", argv[0]);
    return 1;
  }

  auto buf = loadFile(argv[1]);

  printf(
    "%%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) "
    "<< 24))\n");
  printf("\n");

  BoxReader topReader;
  topReader.br = { buf.data(), (int)buf.size() };
  topReader.myBox.size = buf.size();
  topReader.myBox.fourcc = FOURCC("root");
  topReader.specs = g_allSpecs();
  auto parseFunc = getParseFunction(topReader.myBox.fourcc);

  std::string fnStr(argv[1]);
  auto const extPos = fnStr.find_last_of('.');
  if(
    extPos == std::string::npos ||
    (fnStr.substr(extPos) != ".obu" && fnStr.substr(extPos) != ".av1" && fnStr.substr(extPos) != ".av1b")) {
    parseFunc(&topReader);

    for(auto &child : topReader.myBox.children)
      dump(child);
  } else {
    // AV1 bitstream
    while(!topReader.br.empty()) {
      Av1State stateUnused;
      parseAv1Obus(&topReader, stateUnused, true);
    }

    topReader.myBox.fourcc = 0; // put an empty label to remove the top box
    dump(topReader.myBox);
  }

  printf("\n; vim: syntax=nasm\n");

  return 0;
}
