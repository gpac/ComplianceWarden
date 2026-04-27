#include "iamf_utils.h"

#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"

#include <memory>

#include "bit_reader_utils.h"

namespace
{

void parseIASequenceHeaderOBU(ReaderBits *br, IamfState &state)
{
  state.ia_code = br->sym("ia_code", 32);
  br->sym("primary_profile", 8);
  br->sym("additional_profile", 8);
}

} // namespace

int64_t parseIamfObus(IReader *br, IamfState &state)
{
  br->sym("obu", 0); // virtual OBU separator
  auto obu_type = br->sym("obu_type", 5);
  br->sym("obu_redundant_copy", 1);
  state.obu_trimming_status_flag = br->sym("obu_trimming_status_flag", 1);
  auto obu_extension_flag = br->sym("obu_extension_flag", 1);

  uint64_t obuSize = leb128_read(br);

  auto brBits = std::make_unique<ReaderBits>(br);

  if(state.obu_trimming_status_flag) {
    leb128_read(brBits.get()); // num_samples_to_trim_at_end
    leb128_read(brBits.get()); // num_samples_to_trim_at_start
  }

  if(obu_extension_flag) {
    uint64_t extension_header_size = leb128_read(brBits.get());
    for(uint64_t i = 0; i < extension_header_size; ++i) {
      brBits->sym("extension_header_byte", 8);
    }
  }

  switch(obu_type) {
  case OBU_IA_Sequence_Header:
    br->sym("seqhdr", 0);
    parseIASequenceHeaderOBU(brBits.get(), state);
    br->sym("/seqhdr", 0);
    state.seenSequenceHeader = true;
    break;

  default:
    break;
  }

  state.obuCount++;

  int payloadBytesRead = brBits->count / 8;
  int remainingBytes = obuSize - payloadBytesRead;

  while(remainingBytes-- > 0) {
    if(br->empty()) {
      fprintf(stderr, "Incomplete OBU (remaining to read=%d)\n", remainingBytes + 1);
      return obu_type;
    }
    auto boxReader = dynamic_cast<BoxReader *>(br);
    if(boxReader) {
      boxReader->br.m_pos += 8;
    }
  }

  return obu_type;
}

void validateSequenceHeaderTrimming(const IamfState &state, IReport *out)
{
  if(state.seenSequenceHeader) {
    if(state.obu_trimming_status_flag != 0) {
      out->error(
        "[Section 3.2] obu_trimming_status_flag SHALL be 0 for IA Sequence Header OBU, found %d",
        state.obu_trimming_status_flag);
    }
  }
}

void validateSequenceHeaderIaCode(const IamfState &state, IReport *out)
{
  if(state.seenSequenceHeader) {
    if(state.ia_code != FOURCC("iamf")) {
      out->error("[Section 3.4] ia_code is a 'four-character code' (4CC), 'iamf', found 0x%08X", state.ia_code);
    }
  }
}

void validateFirstObuIsSeqHdr(const IamfState &state, IReport *out)
{
  if(!state.seenSequenceHeader) {
    out->error("[Section 3.4] The first OBU in an IA Sequence SHALL have obu_type = OBU_IA_Sequence_Header");
  }
}
