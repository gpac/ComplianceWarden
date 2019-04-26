#include <cstdint>
#include <cstdio>
#include <cassert>
#include <vector>

using namespace std;

struct Box
{
  uint32_t fourcc;
  vector<Box> children;

  void add(const char* name, int64_t value)
  {
    if(0)
      printf("%s: %d\n", name, (int)value);
  }
};

struct BitReader
{
  const uint8_t* src;
  const int size = 0;

  int64_t u(int n)
  {
    uint64_t r = 0;

    for(int i = 0; i < n; ++i)
      r = (r << 1) | bit();

    return r;
  }

  int bit()
  {
    const int byteOffset = m_pos / 8;
    const int bitOffset = m_pos % 8;

    assert(byteOffset < size);

    m_pos++;
    return (src[byteOffset] >> (7 - bitOffset)) & 1;
  }

  bool empty() const
  {
    return m_pos / 8 >= size;
  }

  int m_pos = 0;
};

template<size_t N>
constexpr uint32_t FOURCC(const char(&tab)[N])
{
  static_assert(N == 4 + 1);
  uint32_t r = 0;

  for(int i = 0; i < 4; ++i)
  {
    r <<= 8;
    r |= tab[i];
  }

  return r;
}

using ParseBoxFunc = void(BitReader & br, Box & box);
ParseBoxFunc* getParseFunction(uint32_t fourcc);

void parseRaw(BitReader& br, Box& box)
{
  while(!br.empty())
    br.u(8);

  (void)box;
  // while(!br.empty())
  // box.add("raw_byte", br.u(8));
}

void parseMfhd(BitReader& br, Box& box)
{
  box.add("version", br.u(8));
  box.add("flags", br.u(24));
  box.add("sequence_number", br.u(32));
}

void parseMvhd(BitReader& br, Box& box)
{
  box.add("version", br.u(8));
  box.add("flags", br.u(24));
  box.add("creation_time", br.u(32));
  box.add("modification_time", br.u(32));
  box.add("timescale", br.u(32));
  box.add("duration", br.u(32));
}

void parseTfdt(BitReader& br, Box& box)
{
  box.add("version", br.u(8));
  box.add("flags", br.u(24));
  box.add("base_media_decode_time", (long long)br.u(64));
}

void parseTrun(BitReader& br, Box& box)
{
  box.add("version", br.u(8));
  auto flags = br.u(24);
  box.add("flags", flags);
  // 0x000001 data‐offset‐present
  // 0x000004 first‐sample‐flags‐present
  // 0x000100 sample‐duration‐present
  // 0x000200 sample‐size‐present
  // 0x000400 sample‐flags‐present
  // 0x000800 sample‐composition‐time‐offsets‐present
  auto const sample_count = br.u(32);
  box.add("sample_count", sample_count);

  if(flags & 0x1)
    box.add("data_offset", br.u(32));

  if(flags & 0x4)
    box.add("first_sample_flags", br.u(32));

  for(int i = 0; i < sample_count; ++i)
  {
    if(flags & 0x100)
      box.add("sample_duration", br.u(32));

    if(flags & 0x200)
      box.add("sample_size", br.u(32));

    if(flags & 0x400)
      box.add("sample_flags", br.u(32));

    if(flags & 0x800)
      box.add("sample_composition_time_offset", br.u(32));
  }
}

void parseChildren(BitReader& br, Box& box)
{
  while(!br.empty())
  {
    Box child;
    const uint32_t size = br.u(32);
    child.fourcc = br.u(32);

    const auto payloadSize = int(size - 8);
    auto sub = BitReader { br.src + br.m_pos / 8, payloadSize };
    auto parseFunc = getParseFunction(child.fourcc);
    parseFunc(sub, child);
    box.children.push_back(std::move(child));
    br.m_pos += payloadSize * 8;
  }
}

ParseBoxFunc* getParseFunction(uint32_t fourcc)
{
  switch(fourcc)
  {
  case FOURCC("moov"):
  case FOURCC("trak"):
  case FOURCC("moof"):
  case FOURCC("traf"):
  case FOURCC("ilst"):
  case FOURCC("meta"):
  case FOURCC("mvex"):
  case FOURCC("mdia"):
  case FOURCC("minf"):
  case FOURCC("stbl"):
  case FOURCC("dref"):
  case FOURCC(".too"):
    return &parseChildren;
  case FOURCC("mdat"):
    return &parseRaw;
  case FOURCC("mfhd"):
    return &parseMfhd;
  case FOURCC("mvhd"):
    return &parseMvhd;
  case FOURCC("tfdt"):
    return &parseTfdt;
  case FOURCC("trun"):
    return &parseTrun;
  }

  return &parseRaw;
}

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

  for(auto& child : box.children)
    dump(child, depth + 1);
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
  parseChildren(br, root);
  dump(root);
  return 0;
}

