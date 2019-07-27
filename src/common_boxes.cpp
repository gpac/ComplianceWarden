// Box-parsers for MP4 and MPEG-DASH
#include "common_boxes.h"

namespace
{
void parseRaw(IReader* br)
{
  while(!br->empty())
    br->sym("", 8);
}

void parseFtyp(IReader* br)
{
  br->sym("brand", 32);
  br->sym("version", 32);

  while(!br->empty())
    br->sym("compatible_brand", 32);
}

void parseMeta(IReader* br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty())
    br->box();
}

void parseIloc(IReader* br)
{
  auto version = br->sym("version", 8);

  br->sym("flags", 24);

  br->sym("offset_size", 4);
  br->sym("length_size", 4);
  br->sym("base_offset_size", 4);

  if((version == 1) || (version == 2))
    br->sym("index_size", 4);

  else
    br->sym("reserved1", 4);

  int64_t item_count = 0;

  if(version < 2)
    item_count = br->sym("item_count", 16);

  else if(version == 2)
    item_count = br->sym("item_count", 32);

  for(auto i = 0; i < item_count; i++)
  {
    if(version < 2)
      br->sym("item_ID", 16);

    else if(version == 2)
      br->sym("item_ID", 32);

    if((version == 1) || (version == 2))
    {
      br->sym("reserved2", 12);
      br->sym("construction_method", 4);
    }
  }

  while(!br->empty())
    br->sym("", 8);
}

void parseIinf(IReader* br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty())
    br->sym("", 8);
}

void parseHdlr(IReader* br)
{
  br->sym("version", 8);
  br->sym("flags", 24);
  br->sym("pre_defined", 32);
  br->sym("handler_type", 32);
  br->sym("reserved1", 32);
  br->sym("reserved2", 32);
  br->sym("reserved3", 32);

  while(!br->empty())
    br->sym("", 8);
}

void parseClli(IReader* br)
{
  br->sym("max_content_light_level", 16);
  br->sym("max_pic_average_light_level", 16);
}

void parseMdcv(IReader* br)
{
  br->sym("display_primaries_x_0", 16);
  br->sym("display_primaries_y_0", 16);
  br->sym("display_primaries_x_1", 16);
  br->sym("display_primaries_y_1", 16);
  br->sym("display_primaries_x_2", 16);
  br->sym("display_primaries_y_2", 16);
  br->sym("white_point_x", 16);
  br->sym("white_point_y", 16);
  br->sym("max_display_mastering_luminance", 32);
  br->sym("min_display_mastering_luminance", 32);
}

void parseChildren(IReader* br)
{
  while(!br->empty())
    br->box();
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
  case FOURCC("mvex"):
  case FOURCC("mdia"):
  case FOURCC("minf"):
  case FOURCC("stbl"):
  case FOURCC("dref"):
  case FOURCC(".too"):
  case FOURCC("ipco"):
  case FOURCC("iprp"):
    return &parseChildren;
  case FOURCC("ftyp"):
    return &parseFtyp;
  case FOURCC("meta"):
    return &parseMeta;
  case FOURCC("iloc"):
    return &parseIloc;
  case FOURCC("iinf"):
    return &parseIinf;
  case FOURCC("hdlr"):
    return &parseHdlr;
  case FOURCC("mdat"):
    return &parseRaw;
  case FOURCC("clli"):
    return &parseClli;
  case FOURCC("mdcv"):
    return &parseMdcv;
  }

  return &parseRaw;
}

