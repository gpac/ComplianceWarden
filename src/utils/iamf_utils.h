#pragma once

#include "core/box_reader.h"

#include <cstdint>
#include <vector>

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

struct CodecConfigInfo {
  uint64_t codec_config_id;
  uint32_t codec_id;
  uint64_t num_samples_per_frame;
  int16_t audio_roll_distance;
};

struct IamfState {
  // General
  bool seenSequenceHeader = false;
  int obuCount = 0;
  int obu_trimming_status_flag = -1; // Value from the current OBU
  uint32_t ia_code = 0;

  // IA Sequence Header
  bool firstObuIsSeqHdr = false;
  int seq_hdr_obu_trimming_status_flag = -1;

  // Codec Config
  std::vector<CodecConfigInfo> codecConfigs;
};

int64_t parseIamfObus(IReader *br, IamfState &state);
void validateFirstObuIsSeqHdr(const IamfState &state, IReport *out);
void validateSequenceHeaderTrimming(const IamfState &state, IReport *out);
void validateSequenceHeaderIaCode(const IamfState &state, IReport *out);
void validateCodecConfig(const IamfState &state, IReport *out);
void parseIacb(IReader *br);
