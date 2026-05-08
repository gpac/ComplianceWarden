#pragma once

#include "core/box_reader.h"

#include <cstdint>
#include <string>
#include <vector>

enum {
  OBU_IA_Codec_Config = 0,
  OBU_IA_Audio_Element = 1,
  OBU_IA_Mix_Presentation = 2,
  OBU_IA_Parameter_Block = 3,
  OBU_IA_Temporal_Delimiter = 4,
  OBU_IA_Audio_Frame = 5,
  OBU_IA_Audio_Frame_ID0 = 6,
  OBU_IA_Audio_Frame_ID1 = 7,
  OBU_IA_Audio_Frame_ID2 = 8,
  OBU_IA_Audio_Frame_ID3 = 9,
  OBU_IA_Audio_Frame_ID4 = 10,
  OBU_IA_Audio_Frame_ID5 = 11,
  OBU_IA_Audio_Frame_ID6 = 12,
  OBU_IA_Audio_Frame_ID7 = 13,
  OBU_IA_Audio_Frame_ID8 = 14,
  OBU_IA_Audio_Frame_ID9 = 15,
  OBU_IA_Audio_Frame_ID10 = 16,
  OBU_IA_Audio_Frame_ID11 = 17,
  OBU_IA_Audio_Frame_ID12 = 18,
  OBU_IA_Audio_Frame_ID13 = 19,
  OBU_IA_Audio_Frame_ID14 = 20,
  OBU_IA_Audio_Frame_ID15 = 21,
  OBU_IA_Audio_Frame_ID16 = 22,
  OBU_IA_Audio_Frame_ID17 = 23,
  OBU_IA_Sequence_Header = 31
};

struct IReader;
struct IReport;

enum { AUDIO_ELEMENT_CHANNEL_BASED = 0, AUDIO_ELEMENT_SCENE_BASED = 1 };

enum { PARAMETER_DEFINITION_MIX_GAIN = 0, PARAMETER_DEFINITION_DEMIXING = 1, PARAMETER_DEFINITION_RECON_GAIN = 2 };

struct ParamDefinition {
  uint64_t parameter_id;
  uint64_t parameter_rate;
  uint8_t param_definition_mode;
  uint64_t duration = 0;
  uint64_t constant_subblock_duration = 0;
  uint64_t num_subblocks = 0;
  std::vector<uint64_t> subblock_durations;
};

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
  std::vector<ParamDefinition> param_definitions;
  // For CHANNEL_BASED
  ScalableChannelLayoutConfig scalable_channel_layout_config;
  // For SCENE_BASED
  AmbisonicsConfig ambisonics_config;
};

struct RenderingConfig {
  uint8_t headphones_rendering_mode;
};

struct MixGainParamDefinition : ParamDefinition {
  int16_t default_mix_gain;
};

struct Layout {
  uint8_t layout_type;
  uint8_t sound_system; // only valid if layout_type == 2
};

struct AnchoredLoudness {
  uint8_t anchor_element;
  int16_t anchored_loudness;
};

struct LoudnessInfo {
  uint8_t info_type = 0;
  int16_t integrated_loudness = 0;
  int16_t digital_peak = 0;
  int16_t true_peak = 0; // valid if info_type & 1
  std::vector<AnchoredLoudness> anchored_loudnesses; // valid if info_type & 2
};

struct SubMixAudioElementInfo {
  uint64_t audio_element_id;
  RenderingConfig rendering_config;
  MixGainParamDefinition element_mix_gain;
};

struct SubMixInfo {
  std::vector<SubMixAudioElementInfo> audio_elements;
  MixGainParamDefinition output_mix_gain;
  std::vector<std::pair<Layout, LoudnessInfo>> layouts;
};

struct MixPresentationInfo {
  uint64_t mix_presentation_id;
  std::vector<SubMixInfo> sub_mixes;
  std::vector<std::pair<std::string, std::string>> mix_presentation_tags;
};

struct ParameterBlockInfo {
  uint64_t parameter_id;
  uint64_t duration;
  uint64_t constant_subblock_duration;
  uint64_t num_subblocks;
  std::vector<uint64_t> subblock_durations;
};

struct AudioFrameInfo {
  uint64_t obu_type;
  uint64_t substream_id;
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

  // Mix Presentation
  std::vector<MixPresentationInfo> mixPresentations;

  // Parameter Block
  std::vector<ParameterBlockInfo> parameterBlocks;

  // Audio Frame
  std::vector<AudioFrameInfo> audioFrames;
};

int64_t parseIamfObus(IReader *br, IamfState &state);
void validateFirstObuIsSeqHdr(const IamfState &state, IReport *out);
void validateSequenceHeaderTrimming(const IamfState &state, IReport *out);
void validateSequenceHeaderIaCode(const IamfState &state, IReport *out);
void validateCodecConfig(const IamfState &state, IReport *out);
void validateAudioElement(const IamfState &state, IReport *out);
void validateParameterDefinitions(const IamfState &state, IReport *out);
void validateScalableChannelLayoutConfig(const IamfState &state, IReport *out);
void validateScalableChannelLayoutGeneration(const IamfState &state, IReport *out);
void validateScalableChannelGroupFormat(const IamfState &state, IReport *out);
void validateAmbisonicsConfig(const IamfState &state, IReport *out);
void validateMixPresentation(const IamfState &state, IReport *out);
void validateMixPresentationLoudness(const IamfState &state, IReport *out);
void validateMixPresentationTags(const IamfState &state, IReport *out);
void validateParameterBlocks(const IamfState &state, IReport *out);
void validateAudioFrames(const IamfState &state, IReport *out);
void parseIacb(IReader *br);
