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

enum { AUDIO_ELEMENT_CHANNEL_BASED = 0, AUDIO_ELEMENT_SCENE_BASED = 1 };

enum { PARAMETER_DEFINITION_MIX_GAIN = 0, PARAMETER_DEFINITION_DEMIXING = 1, PARAMETER_DEFINITION_RECON_GAIN = 2 };

struct CodecConfigInfo {
  uint64_t codec_config_id;
  uint32_t codec_id;
  uint64_t num_samples_per_frame;
  int16_t audio_roll_distance;
};

struct ChannelAudioLayerConfig {
  uint8_t loudspeaker_layout;
  uint8_t output_gain_is_present_flag;
  uint8_t recon_gain_is_present_flag;
  uint8_t substream_count;
  uint8_t coupled_substream_count;
  // Present if output_gain_is_present_flag == 1
  uint8_t output_gain_flags;
  int16_t output_gain;
  // Present if loudspeaker_layout == 15 and it's the first layer
  uint8_t expanded_loudspeaker_layout;
};

struct ScalableChannelLayoutConfig {
  uint8_t num_layers;
  std::vector<ChannelAudioLayerConfig> channel_audio_layer_config;
};

struct AmbisonicsMonoConfig {
  uint8_t output_channel_count;
  uint8_t substream_count;
  std::vector<uint8_t> channel_mapping;
};

struct AmbisonicsProjectionConfig {
  uint8_t output_channel_count;
  uint8_t substream_count;
  uint8_t coupled_substream_count;
  std::vector<std::vector<int16_t>> demixing_matrix;
};

struct AmbisonicsConfig {
  uint64_t ambisonics_mode;
  AmbisonicsMonoConfig mono_config;
  AmbisonicsProjectionConfig projection_config;
};

struct AudioElementInfo {
  uint64_t audio_element_id;
  uint8_t audio_element_type;
  bool ignored = false;
  uint64_t codec_config_id;
  uint64_t num_substreams;
  std::vector<uint64_t> audio_substream_ids;
  uint64_t num_parameters;
  std::vector<uint64_t> param_definition_types;
  // For CHANNEL_BASED
  ScalableChannelLayoutConfig scalable_channel_layout_config;
  // For SCENE_BASED
  AmbisonicsConfig ambisonics_config;
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

  // Audio Element
  std::vector<AudioElementInfo> audioElements;
};

int64_t parseIamfObus(IReader *br, IamfState &state);
void validateFirstObuIsSeqHdr(const IamfState &state, IReport *out);
void validateSequenceHeaderTrimming(const IamfState &state, IReport *out);
void validateSequenceHeaderIaCode(const IamfState &state, IReport *out);
void validateCodecConfig(const IamfState &state, IReport *out);
void validateAudioElement(const IamfState &state, IReport *out);
void parseIacb(IReader *br);
