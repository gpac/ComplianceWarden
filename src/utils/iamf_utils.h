#pragma once

#include "core/box_reader.h"

#include <cstdint>

enum {
  OBU_IA_Codec_Config = 0,
  OBU_IA_Audio_Element = 1,
  OBU_IA_Mix_Presentation = 2,
  OBU_IA_Parameter_Block = 3,
  OBU_IA_Temporal_Delimiter = 4,
  OBU_IA_Audio_Frame = 5,
  OBU_IA_Sequence_Header = 31
};

struct IReader;
struct IReport;

struct IamfState {
  bool seenSequenceHeader = false;
  int obuCount = 0;
  int obu_trimming_status_flag = -1;
  uint32_t ia_code = 0;
};

int64_t parseIamfObus(IReader *br, IamfState &state);
void validateFirstObuIsSeqHdr(const IamfState &state, IReport *out);
void validateSequenceHeaderTrimming(const IamfState &state, IReport *out);
void validateSequenceHeaderIaCode(const IamfState &state, IReport *out);
