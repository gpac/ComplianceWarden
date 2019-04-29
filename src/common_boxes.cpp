// Box-parsers for MP4 and MPEG-DASH
#include "common_boxes.h"

namespace
{
void parseRaw(BitReader& br, Box& box)
{
  while(!br.empty())
    br.u(8);

  (void)box;
  // while(!br.empty())
  // box.add("raw_byte", br.u(8));
}

void parseFtyp(BitReader& br, Box& box)
{
  box.add("brand", br.u(32));
  box.add("version", br.u(32));

  while(!br.empty())
    box.add("compatible_brand", br.u(32));
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
    auto sub = br.sub(payloadSize);
    auto parseFunc = getParseFunction(child.fourcc);
    parseFunc(sub, child);
    box.children.push_back(std::move(child));
  }
}
}

ParseBoxFunc* getParseFunction(uint32_t fourcc)
{
  switch(fourcc)
  {
  case FOURCC("root"):
    return &parseChildren;
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
  case FOURCC("ftyp"):
    return &parseFtyp;
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

