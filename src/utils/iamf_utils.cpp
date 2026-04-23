#include "iamf_utils.h"
#include "core/box_reader_impl.h"

#include <cstdio>
#include <memory>
#include <stdexcept>

namespace
{
#define READ_UNTIL_NEXT_BYTE(readBits)                                                                                 \
  if(readBits % 8) {                                                                                                   \
    auto remainderBits = 8 - (readBits % 8);                                                                           \
    br->sym("bits", remainderBits);                                                                                    \
    readBits += remainderBits;                                                                                         \
  }

struct ReaderBits : IReader {
  ReaderBits(IReader *delegate)
      : delegate(delegate)
  {
  }

  virtual ~ReaderBits() {}

  bool empty() { return delegate->empty(); }

  int64_t &sym(const char *name, int bits)
  {
    count += bits;
    return delegate->sym(name, bits);
  }

  void box() { delegate->box(); }

  IReader *delegate = nullptr;
  int64_t count = 0;
};

uint64_t leb128_read(IReader *br)
{
  uint64_t value = 0;

  for(int i = 0; i < 8; i++) {
    uint8_t leb128_byte = br->sym("leb128_byte", 8);
    value |= (((uint64_t)(leb128_byte & 0x7f)) << (i * 7));

    if(!(leb128_byte & 0x80))
      break;
  }

  return value;
}

void parseIaSequenceHeader(IReader *reader)
{
  auto br = std::make_unique<ReaderBits>(reader);

  br->sym("ia_code", 32);
  br->sym("primary_profile", 8);
  br->sym("additional_profile", 8);
}

void parseAudioElement(IReader *reader)
{
  auto br = std::make_unique<ReaderBits>(reader);

  /*auto audio_element_id =*/ leb128_read(br.get());
  br->sym("audio_element_type_byte", 8);
}

void parseMixPresentation(IReader *reader)
{
  auto br = std::make_unique<ReaderBits>(reader);

  /*auto mix_presentation_id =*/ leb128_read(br.get());
  auto count_label = leb128_read(br.get());
  
  for(int i=0; i<(int)count_label; i++) {
    while(br->sym("char", 8) != 0) {}
    while(br->sym("char", 8) != 0) {}
  }
  
  br->sym("num_sub_mixes", 8);
}

void parseCodecConfig(IReader *reader)
{
  auto br = std::make_unique<ReaderBits>(reader);

  /*auto codec_config_id =*/ leb128_read(br.get());
  auto codec_id = br->sym("codec_id", 32);
  /*auto num_samples_per_frame =*/ leb128_read(br.get());
  br->sym("audio_roll_distance", 16);
  
  if(codec_id == FOURCC("Opus")) {
    br->sym("opus_version", 8);
    br->sym("output_channel_count", 8);
    br->sym("pre_skip", 16);
    br->sym("input_sample_rate", 32);
    br->sym("output_gain", 16);
    br->sym("channel_mapping_family", 8);
  } else if(codec_id == FOURCC("ipcm")) {
    br->sym("sample_format_flags", 8);
    br->sym("sample_size", 8);
    br->sym("sample_rate", 32);
  }
}

} // anonymous namespace

void parseIamfObus(IReader *br, int64_t size)
{
  auto reader = std::make_unique<ReaderBits>(br);
  
  while(size > 0 && !reader->empty()) {
    reader->sym("obu", 0);
    auto start_bits = reader->count;
    
    auto raw_byte = reader->sym("raw_byte", 8);
    
    auto obu_type = (raw_byte >> 3) & 0x1F;
            
    long long unsigned obuSize = leb128_read(reader.get());
    
    switch(obu_type) {
    case 31: // OBU_IA_SEQUENCE_HEADER
      {
        auto start_pos = reader->count;
        reader->sym("ia_sequence_header", 0);
        parseIaSequenceHeader(reader.get());
        reader->sym("/ia_sequence_header", 0);
        auto consumed_bytes = (reader->count - start_pos) / 8;
        auto remaining = obuSize - consumed_bytes;
        while(remaining-- > 0)
          reader->sym("obu_payload_padding", 8);
      }
      break;
    case 0: // OBU_IA_CODEC_CONFIG
      {
        auto start_pos = reader->count;
        reader->sym("ia_codec_config", 0);
        parseCodecConfig(reader.get());
        reader->sym("/ia_codec_config", 0);
        auto consumed_bytes = (reader->count - start_pos) / 8;
        auto remaining = obuSize - consumed_bytes;
        while(remaining-- > 0)
          reader->sym("obu_payload_padding", 8);
      }
      break;
    case 1: // OBU_IA_AUDIO_ELEMENT
      {
        auto start_pos = reader->count;
        reader->sym("ia_audio_element", 0);
        parseAudioElement(reader.get());
        reader->sym("/ia_audio_element", 0);
        auto consumed_bytes = (reader->count - start_pos) / 8;
        auto remaining = obuSize - consumed_bytes;
        while(remaining-- > 0)
          reader->sym("obu_payload_padding", 8);
      }
      break;
    case 2: // OBU_IA_MIX_PRESENTATION
      {
        auto start_pos = reader->count;
        reader->sym("ia_mix_presentation", 0);
        parseMixPresentation(reader.get());
        reader->sym("/ia_mix_presentation", 0);
        auto consumed_bytes = (reader->count - start_pos) / 8;
        auto remaining = obuSize - consumed_bytes;
        while(remaining-- > 0)
          reader->sym("obu_payload_padding", 8);
      }
      break;
    default:
      {
        auto remaining = obuSize;
        while(remaining-- > 0)
          reader->sym("obu_payload", 8);
      }
      break;
    }
    
    auto end_bits = reader->count;
    auto bits_read = end_bits - start_bits;
    auto bytes_read = bits_read / 8;
    size -= bytes_read;
  }
}
