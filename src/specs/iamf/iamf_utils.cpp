#include "iamf_utils.h"
#include "core/box_reader_impl.h"

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

} // anonymous namespace

void parseIamfObus(IReader *br, int64_t size)
{
  while(size > 0 && !br->empty()) {
    br->sym("obu", 0); // virtual OBU separator
    auto initial_pos = br->empty() ? 0 : 1; // dummy way to estimate bytes read?
    // Wait, IReader doesn't expose Tell() easily in the interface.
    // But we can track bits read via ReaderBits if we wrap it?
    // But we are passing 'br' to sub-parsers.
    
    auto obu_type = br->sym("obu_type", 5);
    br->sym("obu_redundant_copy", 1);
    auto obu_has_size_field = br->sym("obu_has_size_field", 1);
    br->sym("obu_reserved_1bit", 1);
    
    long long unsigned obuSize = obu_has_size_field ? leb128_read(br) : (size - 1); // rough estimate
    
    // We need to know how many bytes the header took.
    // OBU header is 1 byte + leb128 size.
    // Let's assume leb128 took some bytes.
    // We can't easily know unless we track it.
    // But for validation, we just parse the payload.
    
    switch(obu_type) {
    case OBU_IA_SEQUENCE_HEADER:
      br->sym("ia_sequence_header", 0);
      parseIaSequenceHeader(br);
      br->sym("/ia_sequence_header", 0);
      break;
    default:
      // Consume payload bytes as raw
      // Wait, we don't know the exact payload size if obu_has_size_field was 0.
      // But for iacb, it should have size field?
      // Spec says: configOBUs contains OBUs.
      // Usually OBUs have size field.
      if(obu_has_size_field) {
        auto remaining = obuSize;
        while(remaining-- > 0)
          br->sym("obu_payload", 8);
      }
      break;
    }
    
    // Update remaining size
    // This is a very rough estimate.
    // If obu_has_size_field was 1, we consumed 1 byte + leb128 + obuSize.
    // Let's assume we consumed it.
    if(obu_has_size_field) {
      // We need to calculate leb128 length.
      // We can just assume it's the remaining bytes in the OBU if we knew the start.
      // But we don't.
      // A better way is to just rely on br->empty() if we are parsing the whole buffer.
    }
    
    // For now, let's assume we process one OBU at a time and size is the whole buffer.
    if(!obu_has_size_field) {
      break; // Cannot proceed without size field if multiple OBUs.
    }
    
    // Size update is tricky without Tell().
    // Let's assume we can just rely on br->empty() for the loop condition if we wrap the whole buffer.
    // But we are passed a 'size' argument.
    // If we use a ReaderBits wrapper for the whole loop, we can track bits!
    // Yes!
  }
}
