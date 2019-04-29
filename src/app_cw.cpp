#include <cstdint>
#include <cstdio>

#include "box.h"
#include "common_boxes.h"
#include "parser.h"
#include "spec.h"

using namespace std;

vector<uint8_t> loadFile(const char* path)
{
  FILE* fp = fopen(path, "rb");
  vector<uint8_t> buf(100 * 1024 * 1024);
  auto const size = fread(buf.data(), 1, buf.size(), fp);
  fclose(fp);
  buf.resize(size);
  buf.shrink_to_fit();
  return buf;
}

void dump(Box const& box, int depth = 0)
{
  for(int i = 0; i < depth; ++i)
    printf("  ");

  printf("%c%c%c%c",
         (box.fourcc >> 24) & 0xff,
         (box.fourcc >> 16) & 0xff,
         (box.fourcc >> 8) & 0xff,
         (box.fourcc >> 0) & 0xff);
  printf("\n");

  for(auto& sym : box.syms)
  {
    for(int i = 0; i < depth; ++i)
      printf("  ");

    printf("%s: %lld\n", sym.name, (long long)sym.value);
  }

  for(auto& child : box.children)
    dump(child, depth + 1);
}

void checkCompliance(Box const& file, SpecDesc const& spec)
{
  struct Output : IOutput
  {
    void error(const char* fmt, ...) override
    {
      fprintf(stderr, "Error: %s\n", fmt);
    }
  };

  Output out;

  for(auto& rule : spec.rules)
    rule.check(file, &out);
}

int main(int argc, const char* argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "Usage: %s <input.mp4>\n", argv[0]);
    return 1;
  }

  auto buf = loadFile(argv[1]);
  BitReader br { buf.data(), (int)buf.size() };
  Box root {};
  root.fourcc = FOURCC("root");
  auto func = getParseFunction(root.fourcc);
  func(br, root);
  dump(root);

  extern const SpecDesc g_dummySpec;
  checkCompliance(root, g_dummySpec);

  return 0;
}

