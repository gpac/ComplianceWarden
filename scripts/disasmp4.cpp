// Keep this file standalone.
//
// Usage example:
// $ $CXX scripts/disasmp4.cpp -o disasmp4
// $ ./disasmp4 < tests/aac.mp4 > yo.asm
// $ nasm -f bin yo.asm -o test.mp4
//
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cassert>
#include <vector>
#include <string>
#include <set>
#include <stdexcept>

using namespace std;

struct BitReader
{
  const uint8_t* src;
  const int size = 0;

  uint64_t u(int n)
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

    if(byteOffset >= size)
      throw runtime_error("truncated file");

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
    r = ((r << 8) | tab[i]);

  return r;
}

void indent(int depth)
{
  printf("%*s", depth * 4, "");
}

void println(int depth, const char* format, ...)
{
  indent(depth);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
}

using DumpFunc = void(BitReader & br, int depth);

void dumpRaw(BitReader& br, int depth)
{
  if(br.empty())
    return;

  indent(depth);
  printf("db");

  while(!br.empty())
    printf(" 0x%.2X,", br.u(8));

  printf("\n");
}

void dumpMfhd(BitReader& br, int depth)
{
  println(depth, "u32(0x%.8x) ; version and flags", br.u(32));
  println(depth, "u32(%d) ; sequence_number", br.u(32));
}

void dumpMvhd(BitReader& br, int depth)
{
  auto version_and_flags = br.u(32);
  println(depth, "u32(0x%.8x) ; version and flags", version_and_flags);
  println(depth, "u32(%d) ; creation_time", br.u(32));
  println(depth, "u32(%d) ; modification_time", br.u(32));
  println(depth, "u32(%d) ; timescale", br.u(32));
  println(depth, "u32(%d) ; duration", br.u(32));

  while(!br.empty())
    println(depth, "db 0x%.2X", br.u(8));
}

void dumpTfdt(BitReader& br, int depth)
{
  auto version_and_flags = br.u(32);
  println(depth, "u32(0x%.8x) ; version and flags", version_and_flags);
  auto version = (version_and_flags >> 24) & 0xff;

  if(version == 1)
  {
    println(depth, "dq %lld ; base-media-decode-time", (long long)br.u(64));
  }
  else
  {
    println(depth, "u32(%d) ; base-media-decode-time", br.u(32));
  }
}

void dumpTfhd(BitReader& br, int depth)
{
  auto version_and_flags = br.u(32);
  auto const flags = version_and_flags & 0xffffff;

  println(depth, "u32(0x%.8x) ; version and flags", version_and_flags);
  println(depth, "u32(%d) ; track-id", br.u(32));

  if(flags & 0x000001)
    println(depth, "dq %lld ; base-data-offset", (long long)br.u(64));

  if(flags & 0x000002)
    println(depth, "u32(%d) ; sample-description-index", br.u(32));

  if(flags & 0x000008)
    println(depth, "u32(%d) ; default-sample-duration", br.u(32));

  if(flags & 0x000010)
    println(depth, "u32(%d) ; default-sample-size", br.u(32));

  if(flags & 0x000020)
    println(depth, "u32(%d) ; default-sample-flags", br.u(32));
}

void dumpFtyp(BitReader& br, int depth)
{
  println(depth, "u32(0x%.8x) ; major_brand", br.u(32));
  println(depth, "u32(%d) ; minor_version", br.u(32));

  while(!br.empty())
    println(depth, "u32(0x%.8x) ; compatible_brand", br.u(32));
}

void dumpMdhd(BitReader& br, int depth)
{
  auto version_and_flags = br.u(32);
  println(depth, "u32(0x%.8x) ; version and flags", version_and_flags);

  auto const version = (version_and_flags >> 24) & 0xff;

  if(version == 1)
    println(depth, "dq %d ; creation_time", br.u(64));
  else
    println(depth, "u32(%d) ; creation_time", br.u(32));

  if(version == 1)
    println(depth, "dq %d ; modification_time", br.u(64));
  else
    println(depth, "u32(%d) ; modification_time", br.u(32));

  println(depth, "u32(%d) ; timescale", br.u(32));

  if(version == 1)
    println(depth, "dq %d ; duration", br.u(64));
  else
    println(depth, "u32(%d) ; duration", br.u(32));

  println(depth, "u16(%d) ; pad bit + langage code", br.u(16));
  println(depth, "u16(%d) ; pre_defined", br.u(16));
}

void dumpTrun(BitReader& br, int depth)
{
  auto version_and_flags = br.u(32);
  println(depth, "u32(0x%.8x) ; version and flags", version_and_flags);
  auto const flags = (version_and_flags & 0xffffff);
  auto const version = (version_and_flags >> 24) & 0xff;

  // 0x000001 data‐offset‐present
  // 0x000004 first‐sample‐flags‐present
  // 0x000100 sample‐duration‐present
  // 0x000200 sample‐size‐present
  // 0x000400 sample‐flags‐present
  // 0x000800 sample‐composition‐time‐offsets‐present

  auto const sample_count = br.u(32);
  println(depth, "u32(%d) ; sample_count", sample_count);

  if(flags & 0x1)
    println(depth, "u32(%d) ; data_offset", br.u(32));

  if(flags & 0x4)
    println(depth, "u32(0x%.8x); first_sample_flags", br.u(32));

  for(int i = 0; i < sample_count; ++i)
  {
    if(flags & 0x100)
      println(depth, "u32(%d) ; sample_duration", br.u(32));

    if(flags & 0x200)
      println(depth, "u32(%d) ; sample_size", br.u(32));

    if(flags & 0x400)
      println(depth, "u32(0x%.8x) ; sample_flags", br.u(32));

    if(flags & 0x800)
    {
      assert(version == 0);
      println(depth, "u32(%d) ; sample_composition_time_offset", br.u(32));
    }
  }
}

void dumpBoxes(BitReader& br, int depth);

DumpFunc* selectDumpFunction(uint32_t fourcc)
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
    return &dumpBoxes;
  case FOURCC("mdat"):
    return &dumpRaw;
  case FOURCC("mfhd"):
    return &dumpMfhd;
  case FOURCC("mdhd"):
    return &dumpMdhd;
  case FOURCC("mvhd"):
    return &dumpMvhd;
  case FOURCC("tfdt"):
    return &dumpTfdt;
  case FOURCC("tfhd"):
    return &dumpTfhd;
  case FOURCC("trun"):
    return &dumpTrun;
  case FOURCC("ftyp"):
  case FOURCC("styp"):
    return &dumpFtyp;
  }

  return &dumpRaw;
}

string allocLabel(string name)
{
  static set<string> labels;

  string r = name;

  int k = 1;

  while(labels.find(r) != labels.end())
  {
    ++k;

    char buffer[256] {};
    snprintf(buffer, sizeof buffer, "%s%d", name.c_str(), k);
    r = buffer;
  }

  labels.insert(r);

  return r;
}

void dumpBoxes(BitReader& br, int depth)
{
  while(!br.empty())
  {
    const uint32_t size = br.u(32);
    const uint32_t type = br.u(32);

    char styp[5] {};

    for(int i = 0; i < 4; ++i)
      styp[i] += char((type >> (3 - i) * 8) & 0xff);

    string slabel = allocLabel(styp);

    auto label = slabel.c_str();

    printf("%s_start:\n", label);
    printf("u32(%s_end - %s_start)\n", label, label);
    printf("fourcc(\"%s\")\n", label);

    const auto payloadSize = int(size - 8);

    auto sub = BitReader { br.src + br.m_pos / 8, payloadSize };
    auto dumpFunc = selectDumpFunction(type);
    dumpFunc(sub, depth + 1);

    printf("%s_end:\n", label);

    br.m_pos += payloadSize * 8;
  }
}

int main()
{
  vector<uint8_t> buf(1024 * 1024);
  auto const size = fread(buf.data(), 1, buf.size(), stdin);
  assert(size < buf.size());
  buf.resize(size);

  printf("; vim: syntax=nasm\n");
  printf("%%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))\n");
  printf("%%define u32(a) dd ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))\n");
  printf("%%define u16(a) dw ( ((((a)>>8)&0xFF) << 0)  + ((((a)>>0)&0xFF) << 8))\n");
  printf("%%define fourcc(a) db a\n");
  printf("\n");
  BitReader br { buf.data(), (int)buf.size() };
  dumpBoxes(br, 0);

  return 0;
}

