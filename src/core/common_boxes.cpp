// Box-parsers for MP4 and MPEG-DASH
#include "common_boxes.h"

#include "core/fourcc.h"

#include <cassert>
#include <vector>

void parseAv1C(IReader *br);

std::string toString(uint32_t fourcc)
{
  char fourccStr[5] = {};
  snprintf(
    fourccStr, 5, "%c%c%c%c", (fourcc >> 24) & 0xff, (fourcc >> 16) & 0xff, (fourcc >> 8) & 0xff, (fourcc >> 0) & 0xff);
  return fourccStr;
}

bool isMpegAudio(uint8_t oti)
{
  switch(oti) {
  case 0x40:
  case 0x6b:
  case 0x69:
  case 0x66:
  case 0x67:
  case 0x68:
    return true;
  default:
    return false;
  }
}

namespace
{
void parseRaw(IReader *br)
{
  while(!br->empty())
    br->sym("", 8);
}

void parseFtyp(IReader *br)
{
  br->sym("major_brand", 32);
  br->sym("minor_version", 32);

  while(!br->empty())
    br->sym("compatible_brand", 32);
}

void parseTkhd(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  if(version == 1) {
    br->sym("version", 64);
    br->sym("modification_time", 64);
    br->sym("track_ID", 32);
    br->sym("reserved", 32);
    br->sym("duration", 64);
  } else // version==0
  {
    br->sym("creation_time", 32);
    br->sym("modification_time", 32);
    br->sym("track_ID", 32);
    br->sym("reserved", 32);
    br->sym("duration", 32);
  }

  br->sym("reserved", 32);
  br->sym("reserved", 32);
  br->sym("layer", 16);
  br->sym("alternate_group", 16);
  br->sym("volume", 16);
  br->sym("reserved", 16);

  for(auto i = 0; i < 9; ++i)
    br->sym("matrix", 32); // id is 0,0,0x40000000

  br->sym("width", 32); // fixed 16.16
  br->sym("height", 32); // fixed 16.16
}

void parseStsz(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto sample_size = br->sym("sample_size", 32);
  auto sample_count = br->sym("sample_count", 32);

  if(sample_size == 0)
    for(auto i = 1; i <= sample_count; i++)
      br->sym("entry_size", 32);
}

void parseStz2(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  br->sym("reserved", 24);
  auto field_size = br->sym("field_size", 8);
  auto sample_count = br->sym("sample_count", 32);

  for(auto i = 1; i <= sample_count; i++)
    br->sym("entry_size", field_size);
}

void parseStco(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entry_count = br->sym("entry_count", 32);

  for(auto i = 1; i <= entry_count; i++)
    br->sym("chunk_offset", 32);
}

void parseCo64(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entry_count = br->sym("entry_count", 32);

  for(auto i = 1; i <= entry_count; i++)
    br->sym("chunk_offset", 64);
}

void parseReferenceTypeChildren(IReader *br)
{
  while(!br->empty())
    br->sym("track_IDs", 32);
}

void parseElst(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  auto entry_count = br->sym("entry_count", 32);

  for(int64_t i = 1; i <= entry_count; i++) {
    if(version == 1) {
      br->sym("edit_duration", 64);
      br->sym("media_time", 64);
    } else // version==0
    {
      br->sym("edit_duration", 32);
      br->sym("media_time", 32);
    }

    br->sym("media_rate_integer", 16);
    br->sym("media_rate_fraction", 16);
  }
}

void parseStsd(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entryCount = br->sym("entry_count", 32);

  for(auto i = 1; i <= entryCount; i++)
    br->box();
}

void parseStsc(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entryCount = br->sym("entry_count", 32);

  for(auto i = 1; i <= entryCount; i++) {
    br->sym("first_chunk", 32);
    br->sym("samples_per_chunk", 32);
    br->sym("sample_description_index", 32);
  }
}

void parseSdtp(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty()) {
    br->sym("is_leading", 2);
    br->sym("sample_depends_on", 2);
    br->sym("sample_is_depended_on", 2);
    br->sym("sample_has_redundancy", 2);
  }
}

void parseAvcC(IReader *br)
{
  br->sym("configurationVersion", 8);
  auto AVCProfileIndication = br->sym("AVCProfileIndication", 8);
  br->sym("profile_compatibility", 8);
  br->sym("AVCLevelIndication", 8);
  br->sym("reserved7", 6);
  br->sym("lengthSizeMinusOne", 2);

  br->sym("reserved8", 3);
  auto numOfSequenceParameterSets = br->sym("numOfSequenceParameterSets", 5);

  for(auto i = 0; i < numOfSequenceParameterSets; i++) {
    auto sequenceParameterSetLength = br->sym("sequenceParameterSetLength", 16);

    while(sequenceParameterSetLength--)
      br->sym("sequenceParameterSet", 8);
  }

  auto numOfPictureParameterSets = br->sym("numOfPictureParameterSets", 8);

  for(auto i = 0; i < numOfPictureParameterSets; i++) {
    auto pictureParameterSetLength = br->sym("pictureParameterSetLength", 16);

    while(pictureParameterSetLength--)
      br->sym("pictureParameterSet", 8);
  }

  if(AVCProfileIndication != 66 && AVCProfileIndication != 77 && AVCProfileIndication != 88) {
    br->sym("reserved9", 6);
    br->sym("chroma_format", 2);
    br->sym("reserved10", 5);
    br->sym("bit_depth_luma_minus8", 3);
    br->sym("reserved11", 5);
    br->sym("bit_depth_chroma_minus8", 3);

    auto numOfSequenceParameterSetExt = br->sym("numOfSequenceParameterSetExt", 8);

    for(auto i = 0; i < numOfSequenceParameterSetExt; i++) {
      auto sequenceParameterSetExtLength = br->sym("sequenceParameterSetExtLength", 16);

      while(sequenceParameterSetExtLength--)
        br->sym("", 8);
    }
  }

  while(!br->empty())
    br->box(); // optional MPEG4ExtensionDescriptorsBox-es
}

void parseHvcC(IReader *br)
{
  br->sym("configurationVersion", 8);
  br->sym("general_profile_space", 2);
  br->sym("general_tier_flag", 1);
  br->sym("general_profile_idc", 5);
  br->sym("general_profile_compatibility_flags", 32);
  br->sym("general_constraint_indicator_flags", 48);
  br->sym("general_level_idc", 8);
  br->sym("reserved", 4);
  br->sym("min_spatial_segmentation_idc", 12);
  br->sym("reserved", 6);
  br->sym("parallelismType", 2);
  br->sym("reserved", 6);
  br->sym("chroma_format_idc", 2);
  br->sym("reserved", 5);
  br->sym("bit_depth_luma_minus8", 3);
  br->sym("reserved", 5);
  br->sym("bit_depth_chroma_minus8", 3);
  br->sym("avgFrameRate", 16);
  br->sym("constantFrameRate", 2);
  br->sym("numTemporalLayers", 3);
  br->sym("temporalIdNested", 1);
  br->sym("lengthSizeMinusOne", 2);
  auto numOfArrays = br->sym("numOfArrays", 8);

  for(int j = 0; j < numOfArrays; j++) {
    br->sym("array_completeness", 1);
    br->sym("reserved", 1);
    br->sym("NAL_unit_type", 6);
    auto numNalus = br->sym("numNalus", 16);

    for(int i = 0; i < numNalus; i++) {
      auto nalUnitLength = br->sym("nalUnitLength", 16);

      while(nalUnitLength--)
        br->sym("nalUnit", 8);
    }
  }

  while(!br->empty())
    br->box(); // optional MPEG4ExtensionDescriptorsBox-es
}

void processEsDescriptor(IReader *br);
void processDecoderConfigDescriptor(IReader *br);
void processAudioSpecificInfoConfig(IReader *br, int size);

void processDescriptor(IReader *br, uint8_t oti /*0 if unknown/unrelevant*/)
{
  auto parseVlc = [](IReader *br) {
    int val = 0;
    int n = 4;

    while(n--) {
      int c = br->sym("byte", 8);
      val <<= 7;
      val = c & 0x7f;

      if(!(c & 0x80))
        break;
    }

    return val;
  };

  auto const tag = br->sym("tag", 8);
  auto const len = parseVlc(br);
  switch(tag) {
  case 0x03:
    processEsDescriptor(br);
    break;
  case 0x04:
    processDecoderConfigDescriptor(br);
    break;
  case 0x05:

    if(isMpegAudio(oti))
      processAudioSpecificInfoConfig(br, len); // DecoderSpecificInfoDescriptor
    else // skip
      for(int i = 0; i < len; ++i)
        br->sym("byte", 8);

    break;
  default:

    for(int i = 0; i < len; ++i)
      br->sym("byte", 8);

    break;
  }
}

// ISO 14496-1 7.2.6.5.1
void processEsDescriptor(IReader *br)
{
  br->sym("ES_ID", 16);

  auto streamDependenceFlag = br->sym("streamDependenceFlag", 1);
  auto URL_Flag = br->sym("URL_Flag", 1);
  auto OCRstreamFlag = br->sym("OCRstreamFlag", 1);
  br->sym("streamPriority", 5);

  if(streamDependenceFlag)
    br->sym("dependsOn_ES_ID", 16);

  if(URL_Flag)
    assert(0 && "URL_Flag=1 is not implemented");

  if(OCRstreamFlag)
    br->sym("OCR_ES_Id", 16);

  processDescriptor(br, 0);
}

// ISO 14496-1 7.2.6.6.1
void processDecoderConfigDescriptor(IReader *br)
{
  auto objectTypeIndication = br->sym("objectTypeIndication", 8);
  br->sym("streamType", 6);
  br->sym("upStream", 1);
  br->sym("reserved", 1);
  br->sym("bufferSizeDB", 24);
  br->sym("maxBitrate", 32);
  br->sym("avgBitrate", 32);

  processDescriptor(br, objectTypeIndication);
}

void processAudioSpecificInfoConfig(IReader *br, int size)
{
  int readBits = 0;

  // GetAudioObjectType()
  auto audioObjectType = br->sym("audioObjectType", 5);
  readBits += 5;

  if(audioObjectType == 31) {
    br->sym("audioObjectTypeExt", 5); // audioObjectType = 32 + audioObjectTypeExt
    readBits += 5;
  }

  // Skip
  if(readBits % 8) {
    auto remainderBits = 8 - (readBits % 8);
    br->sym("bits", remainderBits);
    readBits += remainderBits;
  }

  for(auto i = readBits / 8; i < size; ++i)
    br->sym("byte", 8);
}

void parseEsds(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty())
    processDescriptor(br, 0);
}

void parseStts(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entryCount = br->sym("entry_count", 32);

  for(auto i = 1; i <= entryCount; i++) {
    br->sym("sample_count", 32);
    br->sym("sample_delta", 32);
  }
}

void parseStss(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entryCount = br->sym("entry_count", 32);

  for(auto i = 1; i <= entryCount; i++)
    br->sym("sample_number", 32);
}

void parsePasp(IReader *br)
{
  br->sym("hSpacing", 32);
  br->sym("vSpacing", 32);
}

void parseAudioSampleEntry(IReader *br)
{
  // SampleEntry
  br->sym("reserved1", 8);
  br->sym("reserved2", 8);
  br->sym("reserved3", 8);
  br->sym("reserved4", 8);
  br->sym("reserved5", 8);
  br->sym("reserved6", 8);
  br->sym("data_reference_index", 16);

  // AudioSampleEntry
  br->sym("entry_version", 16);
  br->sym("reserved", 16);
  br->sym("reserved", 16);
  br->sym("reserved", 16);
  br->sym("channelcount", 16);
  br->sym("samplesize", 16);
  br->sym("pre_defined", 16);
  br->sym("reserved", 16);
  br->sym("samplerate", 32);

  while(!br->empty())
    br->box();
}

void parseVisualSampleEntry(IReader *br)
{
  // SampleEntry
  br->sym("reserved1", 8);
  br->sym("reserved2", 8);
  br->sym("reserved3", 8);
  br->sym("reserved4", 8);
  br->sym("reserved5", 8);
  br->sym("reserved6", 8);
  br->sym("data_reference_index", 16);

  // VisualSampleEntry
  br->sym("pre_defined", 16);
  br->sym("reserved7", 16);
  br->sym("pre_defined1", 32);
  br->sym("pre_defined2", 32);
  br->sym("pre_defined3", 32);
  br->sym("width", 16);
  br->sym("height", 16);
  br->sym("horizresolution", 32);
  br->sym("vertresolution", 32);
  br->sym("reserved8", 32);
  br->sym("frame_count", 16);

  for(int i = 0; i < 32; ++i)
    br->sym("compressorname", 8); // compressorname

  br->sym("depth", 16);
  br->sym("pre_defined4", 16);

  while(!br->empty())
    br->box(); // clap, pasp
}

void parseMeta(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty())
    br->box();
}

void parseAuxc(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(br->sym("aux_type", 8)) {}

  while(!br->empty())
    br->sym("aux_subtype", 8);
}

void parseAuxi(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty())
    br->sym("aux_track_type", 8);
}

void parseCcst(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  br->sym("all_ref_pics_intra", 1);
  br->sym("intra_pred_used", 1);
  br->sym("max_ref_per_pic", 4);
  br->sym("reserved", 26);

  while(!br->empty())
    br->sym("", 8);
}

void parseIloc(IReader *br)
{
  auto version = br->sym("version", 8);

  br->sym("flags", 24);

  auto offset_size = br->sym("offset_size", 4);
  auto length_size = br->sym("length_size", 4);
  auto base_offset_size = br->sym("base_offset_size", 4);

  uint8_t index_size = 0;

  if((version == 1) || (version == 2))
    index_size = br->sym("index_size", 4);
  else
    br->sym("reserved1", 4);

  int64_t item_count = 0;

  if(version < 2)
    item_count = br->sym("item_count", 16);
  else if(version == 2)
    item_count = br->sym("item_count", 32);

  for(auto i = 0; i < item_count; i++) {
    if(version < 2)
      br->sym("item_ID", 16);
    else if(version == 2)
      br->sym("item_ID", 32);

    if((version == 1) || (version == 2)) {
      br->sym("reserved2", 12);
      br->sym("construction_method", 4);
    }

    br->sym("data_reference_index", 16);
    br->sym("base_offset", base_offset_size * 8);
    auto extent_count = br->sym("extent_count", 16);

    for(auto j = 0; j < extent_count; j++) {
      if(((version == 1) || (version == 2)) && (index_size > 0))
        br->sym("item_reference_index", index_size * 8);

      br->sym("extent_offset", offset_size * 8);
      br->sym("extent_length", length_size * 8);
    }
  }
}

void parseIinf(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  uint32_t entry_count;

  if(version == 0)
    entry_count = br->sym("entry_count", 16);
  else
    entry_count = br->sym("entry_count", 32);

  for(uint32_t i = 0; i < entry_count; ++i)
    br->box(); // ItemInfoEntry
}

void parseInfe(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  if((version == 0) || (version == 1)) {
    br->sym("item_ID", 16);
    br->sym("item_protection_index", 16);

    while(br->sym("item_name", 8))
      ; // item_name

    while(br->sym("content_type", 8))
      ; // content_type

    while(br->sym("content_encoding", 8))
      ; // content_encoding
  }

  if(version == 1) {
    while(!br->empty())
      br->sym("ItemInfoExtension(extension_type)", 8);
  } else if(version >= 2) {
    if(version == 2) {
      br->sym("item_ID", 16);
    } else if(version == 3) {
      br->sym("item_ID", 32);
    }

    br->sym("item_protection_index", 16);
    uint32_t item_type = br->sym("item_type", 32);

    while(br->sym("item_name", 8))
      ; // item_name

    if(item_type == FOURCC("mime")) {
      while(br->sym("content_type", 8))
        ; // content_type

      while(!br->empty() && br->sym("content_encoding", 8))
        ; // content_encoding
    } else if(item_type == FOURCC("uri ")) {
      while(br->sym("item_uri_type", 8))
        ; // item_uri_type
    }
  }
}

void parseIspe(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  br->sym("image_width", 32);
  br->sym("image_height", 32);
}

void parseIref(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  if(version == 0) {
    while(!br->empty()) {
      br->sym("box_size", 32);
      br->sym("box_type", 32);
      br->sym("from_item_ID", 16);
      auto reference_count = br->sym("reference_count", 16);

      for(auto j = 0; j < reference_count; j++) {
        br->sym("to_item_ID", 16);
      }
    }
  } else if(version == 1) {
    while(!br->empty()) {
      br->sym("box_size", 32);
      br->sym("box_type", 32);
      br->sym("from_item_ID", 32);
      auto reference_count = br->sym("reference_count", 16);

      for(auto j = 0; j < reference_count; j++) {
        br->sym("to_item_ID", 32);
      }
    }
  }
}

void parseIpma(IReader *br)
{
  auto version = br->sym("version", 8);
  auto flags = br->sym("flags", 24);

  auto entry_count = br->sym("entry_count", 32);

  for(auto i = 0; i < entry_count; i++) {
    if(version < 1)
      br->sym("item_ID", 16);
    else
      br->sym("item_ID", 32);

    auto association_count = br->sym("association_count", 8);

    for(auto i = 0; i < association_count; i++) {
      br->sym("essential", 1);

      if(flags & 1)
        br->sym("property_index", 15);
      else
        br->sym("property_index", 7);
    }
  }
}

void parseHdlr(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);
  br->sym("pre_defined", 32);
  br->sym("handler_type", 32);
  br->sym("reserved1", 32);
  br->sym("reserved2", 32);
  br->sym("reserved3", 32);

  while(!br->empty())
    br->sym("name", 8);
}

void parseDref(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  auto entry_count = br->sym("entry_count", 32);

  for(auto i = 1; i <= entry_count; i++) {
    br->box();
  }
}

void parseUrx(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  while(!br->empty())
    br->sym("byte", 8);
}

void parseClap(IReader *br)
{
  br->sym("cleanApertureWidthN", 32);
  br->sym("cleanApertureWidthD", 32);
  br->sym("cleanApertureHeightN", 32);
  br->sym("cleanApertureHeightD", 32);
  br->sym("horizOffN", 32);
  br->sym("horizOffD", 32);
  br->sym("vertOffN", 32);
  br->sym("vertOffD", 32);
}

void parseTrex(IReader *br)
{
  br->sym("version", 8);
  br->sym("flags", 24);

  br->sym("track_ID", 32);
  br->sym("default_sample_description_index", 32);
  br->sym("default_sample_duration", 32);
  br->sym("default_sample_size", 32);
  br->sym("default_sample_flags", 32);
}

void parseClli(IReader *br)
{
  br->sym("max_content_light_level", 16);
  br->sym("max_pic_average_light_level", 16);
}

void parseMdcv(IReader *br)
{
  for(int i = 0; i < 3; i++) {
    br->sym("display_primaries_x", 16);
    br->sym("display_primaries_y", 16);
  }
  br->sym("white_point_x", 16);
  br->sym("white_point_y", 16);
  br->sym("max_display_mastering_luminance", 32);
  br->sym("min_display_mastering_luminance", 32);
}

void parsePitm(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  if(version == 0)
    br->sym("item_ID", 16);
  else
    br->sym("item_ID", 32);
}

void parseSgpd(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  int64_t default_length = 0;
  int64_t grouping_type = br->sym("grouping_type", 32);
  if(version >= 1) {
    default_length = br->sym("default_length", 32);
  }
  if(version >= 2) {
    br->sym("default_group_description_index", 32);
  }

  auto entry_count = br->sym("entry_count", 32);
  for(int64_t i = 1; i <= entry_count; i++) {
    int64_t description_length = 0;
    if(version >= 1) {
      if(default_length == 0) {
        description_length = br->sym("description_length", 32);
      }
    }

    if(grouping_type == FOURCC("av1M")) {
      br->sym("metadata_type", 8);
      br->sym("metadata_specific_parameters", 24);
    } else {
      auto size = description_length ? description_length : default_length;
      while(size--)
        br->sym("byte", 8);
    }
  }
}

void parseTfhd(IReader *br)
{
  br->sym("version", 8);
  auto flags = br->sym("flags", 24);
  br->sym("track_ID", 32);

  if(flags & 0x000001)
    br->sym("base_data_offset", 64);

  if(flags & 0x000002)
    br->sym("sample_description_index", 32);

  if(flags & 0x000008)
    br->sym("default_sample_duration", 32);

  if(flags & 0x000010)
    br->sym("default_sample_size", 32);

  // default_sample_flags
  if(flags & 0x000020)
    br->sym("default_sample_flags", 32);
}

void parseSbgp(IReader *br)
{
  auto version = br->sym("version", 8);
  br->sym("flags", 24);

  br->sym("grouping_type", 32);
  if(version == 1) {
    br->sym("grouping_type_parameter", 32);
  }

  auto entry_count = br->sym("entry_count", 32);
  for(auto i = 1; i <= entry_count; i++) {
    br->sym("sample_count", 32);
    br->sym("group_description_index", 32);
  }
}

void parseTrun(IReader *br)
{
  auto version = br->sym("version", 8);
  auto flags = br->sym("flags", 24);

  auto sample_count = br->sym("sample_count", 32);

  if(flags & 0x000001)
    br->sym("data_offset", 32);

  if(flags & 0x000004)
    br->sym("first_sample_flags", 32);

  for(auto i = 1; i <= sample_count; i++) {
    if(flags & 0x000100)
      br->sym("sample_duration", 32);

    if(flags & 0x000200)
      br->sym("sample_size", 32);

    if(flags & 0x000400)
      br->sym("sample_flags", 32);

    if(flags & 0x000800) {
      if(version == 0)
        br->sym("sample_composition_time_offset", 32);
      else
        br->sym("sample_composition_time_offset", 32);
    }
  }
}

void parseChildren(IReader *br)
{
  while(!br->empty())
    br->box();
}
}

/*sample entries and item types*/
std::vector<uint32_t> visualSampleEntryFourccs = {
  FOURCC("avc1"), FOURCC("avc2"), FOURCC("avc3"), FOURCC("avc4"), FOURCC("hev1"), FOURCC("hev2"),
  FOURCC("hvc1"), FOURCC("hvc2"), FOURCC("av01"), FOURCC("unci"), FOURCC("uncv"), FOURCC("j2k1"),
};

bool isVisualSampleEntry(uint32_t fourcc)
{
  for(auto se : visualSampleEntryFourccs)
    if(se == fourcc)
      return true;

  return false;
}

ParseBoxFunc *getParseFunction(uint32_t fourcc)
{
  switch(fourcc) {
  case FOURCC("root"):
    return &parseChildren;
  case FOURCC("moov"):
  case FOURCC("trak"):
  case FOURCC("edts"):
  case FOURCC("moof"):
  case FOURCC("tref"):
  case FOURCC("traf"):
  case FOURCC("ilst"):
  case FOURCC("mvex"):
  case FOURCC("mdia"):
  case FOURCC("minf"):
  case FOURCC("dinf"):
  case FOURCC("stbl"):
  case FOURCC(".too"):
  case FOURCC("ipco"):
  case FOURCC("iprp"):
    return &parseChildren;
  case FOURCC("ftyp"):
    return &parseFtyp;
  case FOURCC("tkhd"):
    return &parseTkhd;
  case FOURCC("stco"):
    return &parseStco;
  case FOURCC("co64"):
    return &parseCo64;
  case FOURCC("elst"):
    return &parseElst;
  case FOURCC("stsd"):
    return &parseStsd;
  case FOURCC("stsc"):
    return &parseStsc;
  case FOURCC("sdtp"):
    return &parseSdtp;
  case FOURCC("avcC"):
    return &parseAvcC;
  case FOURCC("hvcC"):
    return &parseHvcC;
  case FOURCC("av1C"):
    return &parseAv1C;
  case FOURCC("pict"):
  case FOURCC("thmb"):
  case FOURCC("auxl"):
    return &parseReferenceTypeChildren;
  case FOURCC("mp4a"):
  case FOURCC("twos"):
    return &parseAudioSampleEntry;
  case FOURCC("esds"):
    return &parseEsds;
  case FOURCC("stts"):
    return &parseStts;
  case FOURCC("stss"):
    return &parseStss;
  case FOURCC("pasp"):
    return &parsePasp;
  case FOURCC("meta"):
    return &parseMeta;
  case FOURCC("tfhd"):
    return &parseTfhd;
  case FOURCC("auxC"):
    return &parseAuxc;
  case FOURCC("auxi"):
    return &parseAuxi;
  case FOURCC("ccst"):
    return &parseCcst;
  case FOURCC("iloc"):
    return &parseIloc;
  case FOURCC("iinf"):
    return &parseIinf;
  case FOURCC("infe"):
    return &parseInfe;
  case FOURCC("ispe"):
    return &parseIspe;
  case FOURCC("iref"):
    return &parseIref;
  case FOURCC("ipma"):
    return &parseIpma;
  case FOURCC("hdlr"):
    return &parseHdlr;
  case FOURCC("dref"):
    return &parseDref;
  case FOURCC("url "):
  case FOURCC("urn "):
    return &parseUrx;
  case FOURCC("mdat"):
    return &parseRaw;
  case FOURCC("clap"):
    return &parseClap;
  case FOURCC("clli"):
    return &parseClli;
  case FOURCC("mdcv"):
    return &parseMdcv;
  case FOURCC("pitm"):
    return &parsePitm;
  case FOURCC("stsz"):
    return &parseStsz;
  case FOURCC("stz2"):
    return &parseStz2;
  case FOURCC("sgpd"):
    return &parseSgpd;
  case FOURCC("sbgp"):
    return &parseSbgp;
  case FOURCC("trex"):
    return &parseTrex;
  case FOURCC("trun"):
    return &parseTrun;
  }

  if(isVisualSampleEntry(fourcc))
    return &parseVisualSampleEntry;

  return &parseRaw;
}
