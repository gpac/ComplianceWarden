#include "iamf_utils.h"

#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"

#include <memory>
#include <set>

#include "bit_reader_utils.h"

namespace
{

void parseIASequenceHeaderOBU(ReaderBits *br, IamfState &state)
{
  state.ia_code = br->sym("ia_code", 32);
  br->sym("primary_profile", 8);
  br->sym("additional_profile", 8);
}

ParamDefinition parseParamDefinition(ReaderBits *br)
{
  ParamDefinition def;
  def.parameter_id = leb128_read(br);
  def.parameter_rate = leb128_read(br);
  def.param_definition_mode = br->sym("param_definition_mode", 1);
  br->sym("reserved", 7);
  if(def.param_definition_mode == 0) {
    def.duration = leb128_read(br);
    def.constant_subblock_duration = leb128_read(br);
    if(def.constant_subblock_duration == 0) {
      def.num_subblocks = leb128_read(br);
      for(uint64_t i = 0; i < def.num_subblocks; i++) {
        def.subblock_durations.push_back(leb128_read(br));
      }
    }
  }
  return def;
}

void parseAudioElementParams(ReaderBits *br, AudioElementInfo &info)
{
  info.num_parameters = leb128_read(br);
  for(uint64_t i = 0; i < info.num_parameters; i++) {
    auto param_definition_type = leb128_read(br);
    info.param_definition_types.push_back(param_definition_type);

    // Parse standard fields only for known parameter types (types <= 2)
    if(param_definition_type <= 2) {
      info.param_definitions.push_back(parseParamDefinition(br));
    }

    if(param_definition_type == PARAMETER_DEFINITION_DEMIXING) {
      br->sym("dmixp_mode", 3);
      br->sym("reserved", 5);
      br->sym("default_w", 4);
      br->sym("reserved", 4);
    } else if(param_definition_type == PARAMETER_DEFINITION_RECON_GAIN) {
      // No additional fields
    } else if(param_definition_type > 2) {
      auto param_definition_size = leb128_read(br);
      for(uint64_t j = 0; j < param_definition_size; j++) {
        br->sym("param_definition_byte", 8);
      }
    }
  }
}

void parseScalableChannelLayoutConfig(ReaderBits *br, AudioElementInfo &info)
{
  info.scalable_channel_layout_config.num_layers = br->sym("num_layers", 3);
  br->sym("reserved", 5);
  for(int i = 1; i <= info.scalable_channel_layout_config.num_layers; i++) {
    ChannelAudioLayerConfig layer;
    layer.loudspeaker_layout = br->sym("loudspeaker_layout", 4);

    if(layer.loudspeaker_layout > 9 && layer.loudspeaker_layout != 15) {
      fprintf(stderr, "Warning: Unknown loudspeaker_layout %d. Skipping remaining layers.\n", layer.loudspeaker_layout);
      break;
    }

    if(i > 1 && layer.loudspeaker_layout == 15) {
      fprintf(stderr, "Warning: loudspeaker_layout 15 found in layer %d. Skipping remaining layers.\n", i);
      break;
    }

    layer.output_gain_is_present_flag = br->sym("output_gain_is_present_flag", 1);
    layer.recon_gain_is_present_flag = br->sym("recon_gain_is_present_flag", 1);
    br->sym("reserved", 2);
    layer.substream_count = br->sym("substream_count", 8);
    layer.coupled_substream_count = br->sym("coupled_substream_count", 8);
    if(layer.output_gain_is_present_flag == 1) {
      layer.output_gain_flags = br->sym("output_gain_flags", 6);
      br->sym("reserved", 2);
      layer.output_gain = br->sym("output_gain", 16);
    }
    if(i == 1 && layer.loudspeaker_layout == 15) {
      layer.expanded_loudspeaker_layout = br->sym("expanded_loudspeaker_layout", 8);
    }
    info.scalable_channel_layout_config.channel_audio_layer_config.push_back(layer);
  }
}

void parseAmbisonicsConfig(ReaderBits *br, AudioElementInfo &info)
{
  auto &ambisonics_config = info.ambisonics_config;
  ambisonics_config.ambisonics_mode = leb128_read(br);
  if(ambisonics_config.ambisonics_mode == 0) { // MONO
    auto &mono_config = ambisonics_config.mono_config;
    mono_config.output_channel_count = br->sym("output_channel_count", 8);
    mono_config.substream_count = br->sym("substream_count", 8);
    for(int j = 0; j < mono_config.output_channel_count; j++) {
      mono_config.channel_mapping.push_back(br->sym("channel_mapping", 8));
    }
  } else if(ambisonics_config.ambisonics_mode == 1) { // PROJECTION
    auto &projection_config = ambisonics_config.projection_config;
    projection_config.output_channel_count = br->sym("output_channel_count", 8);
    projection_config.substream_count = br->sym("substream_count", 8);
    projection_config.coupled_substream_count = br->sym("coupled_substream_count", 8);
    int rows = projection_config.substream_count + projection_config.coupled_substream_count;
    int cols = projection_config.output_channel_count;
    projection_config.demixing_matrix.resize(rows, std::vector<int16_t>(cols));
    for(int r = 0; r < rows; r++) {
      for(int c = 0; c < cols; c++) {
        projection_config.demixing_matrix[r][c] = br->sym("demixing_matrix_element", 16);
      }
    }
  }
}

void parseAudioElementOBU(ReaderBits *br, AudioElementInfo &info)
{
  info.audio_element_id = leb128_read(br);
  info.audio_element_type = br->sym("audio_element_type", 3);
  br->sym("reserved", 5);

  if(info.audio_element_type > 1) {
    fprintf(stderr, "Warning: Unknown audio_element_type %d. Ignoring Audio Element OBU.\n", info.audio_element_type);
    info.ignored = true;
    return;
  }

  info.codec_config_id = leb128_read(br);
  info.num_substreams = leb128_read(br);
  for(uint64_t i = 0; i < info.num_substreams; i++) {
    info.audio_substream_ids.push_back(leb128_read(br));
  }

  parseAudioElementParams(br, info);

  if(info.audio_element_type == AUDIO_ELEMENT_CHANNEL_BASED) {
    parseScalableChannelLayoutConfig(br, info);
  } else if(info.audio_element_type == AUDIO_ELEMENT_SCENE_BASED) {
    parseAmbisonicsConfig(br, info);
  } else {
    auto audio_element_config_size = leb128_read(br);
    for(uint64_t j = 0; j < audio_element_config_size; j++) {
      br->sym("audio_element_config_byte", 8);
    }
  }
}
bool getXYZChannelCounts(uint8_t loudspeaker_layout, int &x, int &y, int &z)
{
  switch(loudspeaker_layout) {
  case 0:
    x = 1;
    y = 0;
    z = 0;
    return true; // Mono
  case 1:
    x = 2;
    y = 0;
    z = 0;
    return true; // Stereo
  case 2:
    x = 5;
    y = 1;
    z = 0;
    return true; // 5.1
  case 3:
    x = 5;
    y = 1;
    z = 2;
    return true; // 5.1.2
  case 4:
    x = 5;
    y = 1;
    z = 4;
    return true; // 5.1.4
  case 5:
    x = 7;
    y = 1;
    z = 0;
    return true; // 7.1
  case 6:
    x = 7;
    y = 1;
    z = 2;
    return true; // 7.1.2
  case 7:
    x = 7;
    y = 1;
    z = 4;
    return true; // 7.1.4
  case 8:
    x = 3;
    y = 1;
    z = 2;
    return true; // 3.1.2
  default:
    return false;
  }
}

bool getExpectedSubstreamCountsForScalableLayer(int layout_prev, int layout_curr, int &num_subs, int &num_coupled)
{
  num_subs = 0; // number of substreams
  num_coupled = 0; // number of coupled substreams

  if(layout_prev == -1) {
    switch(layout_curr) {
    case 0:
      num_subs = 1;
      num_coupled = 0;
      return true; // Mono
    case 1:
      num_subs = 1;
      num_coupled = 1;
      return true; // Stereo
    case 2:
      num_subs = 4;
      num_coupled = 2;
      return true; // 5.1
    case 3:
      num_subs = 5;
      num_coupled = 3;
      return true; // 5.1.2
    case 4:
      num_subs = 6;
      num_coupled = 4;
      return true; // 5.1.4
    case 5:
      num_subs = 5;
      num_coupled = 3;
      return true; // 7.1
    case 6:
      num_subs = 6;
      num_coupled = 4;
      return true; // 7.1.2
    case 7:
      num_subs = 7;
      num_coupled = 5;
      return true; // 7.1.4
    case 8:
      num_subs = 4;
      num_coupled = 2;
      return true; // 3.1.2
    default:
      return false;
    }
  }

  int x_prev, y_prev, z_prev;
  int x_curr, y_curr, z_curr;
  if(!getXYZChannelCounts(layout_prev, x_prev, y_prev, z_prev))
    return false;
  if(!getXYZChannelCounts(layout_curr, x_curr, y_curr, z_curr))
    return false;

  int dx = x_curr - x_prev;
  int dy = y_curr - y_prev;
  int dz = z_curr - z_prev;

  if(dx > 0) {
    if(x_prev == 2 && x_curr == 5) {
      num_subs += 2;
      num_coupled += 1;
    } else if(x_prev == 5 && x_curr == 7) {
      num_subs += 1;
      num_coupled += 1;
    } else if(x_prev == 1 && x_curr == 2) {
      num_subs += 1;
      num_coupled += 0;
    } else if(x_prev == 3 && x_curr == 5) {
      num_subs += 1;
      num_coupled += 1;
    } else {
      return false;
    }
  }

  if(dy > 0) {
    num_subs += 1;
    num_coupled += 0;
  }

  if(dz > 0) {
    if(dz == 2) {
      num_subs += 1;
      num_coupled += 1;
    } else if(dz == 4) {
      num_subs += 2;
      num_coupled += 2;
    } else {
      return false;
    }
  }

  return true;
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
    if(state.obuCount == 0) {
      state.firstObuIsSeqHdr = true;
    }
    if(state.seq_hdr_obu_trimming_status_flag == -1) {
      state.seq_hdr_obu_trimming_status_flag = state.obu_trimming_status_flag;
    }
    break;

  case OBU_IA_Codec_Config: {
    br->sym("codec_config", 0);
    uint64_t codec_config_id = leb128_read(brBits.get());
    uint32_t codec_id = brBits->sym("codec_id", 32);
    uint64_t num_samples_per_frame = leb128_read(brBits.get());
    int16_t audio_roll_distance = brBits->sym("audio_roll_distance", 16);

    state.codecConfigs.push_back({ codec_config_id, codec_id, num_samples_per_frame, audio_roll_distance });

    br->sym("/codec_config", 0);
    break;
  }

  case OBU_IA_Audio_Element: {
    br->sym("audio_element", 0);
    AudioElementInfo info;
    parseAudioElementOBU(brBits.get(), info);
    state.audioElements.push_back(info);
    br->sym("/audio_element", 0);
    break;
  }

  default:
    break;
  }

  state.obuCount++;

  int64_t remainingBits = (obuSize * 8) - brBits->count;
  ENSURE(remainingBits >= 0, "Read %lld bits more than reported OBU size", (long long)(-remainingBits));
  auto boxReader = dynamic_cast<BoxReader *>(br);
  if(boxReader) {
    ENSURE(boxReader->br.m_pos + remainingBits <= boxReader->br.size * 8, "OBU size exceeds box size");
    boxReader->br.m_pos += remainingBits;
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

void validateCodecConfig(const IamfState &state, IReport *out)
{
  std::set<uint64_t> unique_ids;
  for(auto const &config : state.codecConfigs) {
    if(!unique_ids.insert(config.codec_config_id).second) {
      out->error(
        "[Section 3.5] There SHALL be exactly one Codec Config OBU with a given identifier in a set of Descriptors.");
    }

    if(
      config.codec_id != FOURCC("Opus") && config.codec_id != FOURCC("mp4a") && config.codec_id != FOURCC("fLaC") &&
      config.codec_id != FOURCC("ipcm")) {
      out->error("[Section 3.5] codec_id identifies an unsupported codec. Supported values: Opus, mp4a, fLaC, ipcm.");
    }

    if(config.num_samples_per_frame == 0) {
      out->error("[Section 3.5] num_samples_per_frame SHALL NOT be set to zero.");
    }

    if(config.audio_roll_distance > 0) {
      out->error(
        "[Section 3.5] audio_roll_distance SHALL always be a negative value or zero, found %d",
        config.audio_roll_distance);
    }

    if(config.codec_id == FOURCC("Opus")) {
      int16_t expected_roll = -((3840 + config.num_samples_per_frame - 1) / config.num_samples_per_frame);
      if(config.audio_roll_distance != expected_roll) {
        out->error(
          "[Section 3.5] audio_roll_distance SHALL be %d for Opus with num_samples_per_frame = %lu, found %d",
          expected_roll, config.num_samples_per_frame, config.audio_roll_distance);
      }
    }

    if(config.codec_id == FOURCC("mp4a")) {
      if(config.audio_roll_distance != -1) {
        out->error("[Section 3.5] audio_roll_distance SHALL be -1 for mp4a, found %d", config.audio_roll_distance);
      }
    }

    if(config.codec_id == FOURCC("fLaC") || config.codec_id == FOURCC("ipcm")) {
      if(config.audio_roll_distance != 0) {
        out->error(
          "[Section 3.5] audio_roll_distance SHALL be 0 for fLaC or ipcm, found %d", config.audio_roll_distance);
      }
    }
  }
}

void validateAudioElement(const IamfState &state, IReport *out)
{
  std::set<uint64_t> unique_ids;
  for(auto const &elem : state.audioElements) {
    if(!unique_ids.insert(elem.audio_element_id).second) {
      out->error("[Section 3.6] audio_element_id SHALL be unique within an IA Sequence.");
    }

    if(elem.ignored)
      continue;

    if(elem.num_substreams == 0) {
      out->error("[Section 3.6] num_substreams SHALL NOT be set to 0.");
    }

    if(elem.audio_element_type == AUDIO_ELEMENT_CHANNEL_BASED) {
      if(elem.num_parameters > 2) {
        out->error(
          "[Section 3.6] When audio_element_type = 0, num_parameters SHALL be set to 0, 1, or 2. Found %lu",
          elem.num_parameters);
      }
    } else if(elem.audio_element_type == AUDIO_ELEMENT_SCENE_BASED) {
      if(elem.num_parameters != 0) {
        out->error(
          "[Section 3.6] When audio_element_type = 1, num_parameters SHALL be set to 0. Found %lu",
          elem.num_parameters);
      }
    }

    std::set<uint64_t> param_types;
    bool has_mix_gain = false;
    bool has_demixing = false;
    bool has_recon_gain = false;

    for(auto type : elem.param_definition_types) {
      if(type == PARAMETER_DEFINITION_MIX_GAIN)
        has_mix_gain = true;
      if(type == PARAMETER_DEFINITION_DEMIXING)
        has_demixing = true;
      if(type == PARAMETER_DEFINITION_RECON_GAIN)
        has_recon_gain = true;

      if(!param_types.insert(type).second) {
        out->error("[Section 3.6] The parameter type SHALL NOT be duplicated in one Audio Element OBU.");
      }
    }

    if(has_mix_gain) {
      out->error("[Section 3.6] The type PARAMETER_DEFINITION_MIX_GAIN SHALL NOT be present in Audio Element OBU.");
    }

    uint32_t codec_id = 0;
    for(auto const &config : state.codecConfigs) {
      if(config.codec_config_id == elem.codec_config_id) {
        codec_id = config.codec_id;
        break;
      }
    }

    if(codec_id == FOURCC("fLaC") || codec_id == FOURCC("ipcm")) {
      if(has_recon_gain) {
        out->error(
          "[Section 3.6] When codec_id = fLaC or ipcm, the type PARAMETER_DEFINITION_RECON_GAIN SHALL NOT be present.");
      }
    }

    if(elem.audio_element_type == AUDIO_ELEMENT_CHANNEL_BASED) {
      if(elem.scalable_channel_layout_config.num_layers > 1) {
        if(codec_id != FOURCC("fLaC") && codec_id != FOURCC("ipcm")) {
          if(!has_recon_gain) {
            out->error("[Section 3.6] When num_layers > 1, the type PARAMETER_DEFINITION_RECON_GAIN SHALL be present.");
          }
        }
      }

      uint8_t highest_layout = 0;
      if(!elem.scalable_channel_layout_config.channel_audio_layer_config.empty()) {
        highest_layout = elem.scalable_channel_layout_config.channel_audio_layer_config.back().loudspeaker_layout;
      }

      if(elem.scalable_channel_layout_config.num_layers > 1) {
        if(highest_layout != 0 && highest_layout != 1 && highest_layout != 8) {
          if(!has_demixing) {
            out->error(
              "[Section 3.6] When the highest loudspeaker_layout of the scalable channel audio (i.e., num_layers > 1) "
              "is greater than 3.1.2ch, PARAMETER_DEFINITION_DEMIXING SHALL be present.");
          }
          if(codec_id != FOURCC("fLaC") && codec_id != FOURCC("ipcm")) {
            if(!has_recon_gain) {
              out->error(
                "[Section 3.6] When the highest loudspeaker_layout of the scalable channel audio (i.e., num_layers > "
                "1) is greater than 3.1.2ch, PARAMETER_DEFINITION_RECON_GAIN SHALL be present.");
            }
          }
        }
      }
    }
  }
}

void validateParameterDefinitions(const IamfState &state, IReport *out)
{
  std::set<uint64_t> unique_parameter_ids;
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;

    for(auto const &param : elem.param_definitions) {
      if(!unique_parameter_ids.insert(param.parameter_id).second) {
        out->error(
          "[Section 3.6.1] There SHALL be one unique parameter_id per Parameter Substream. Duplicate id %lu",
          param.parameter_id);
      }

      if(param.param_definition_mode == 0) {
        if(param.duration == 0) {
          out->error("[Section 3.6.1] duration SHALL NOT be set to 0 when param_definition_mode is 0.");
        }

        if(param.constant_subblock_duration == 0) {
          uint64_t sum_durations = 0;
          for(auto dur : param.subblock_durations) {
            if(dur == 0) {
              out->error("[Section 3.6.1] subblock_duration SHALL NOT be set to 0.");
            }
            sum_durations += dur;
          }
          if(sum_durations != param.duration) {
            out->error(
              "[Section 3.6.1] The summation of all subblock_duration (%lu) SHALL be equal to duration (%lu).",
              sum_durations, param.duration);
          }
        }
      }
    }
  }
}

void validateScalableChannelLayoutConfig(const IamfState &state, IReport *out)
{
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;
    if(elem.audio_element_type != AUDIO_ELEMENT_CHANNEL_BASED)
      continue;

    auto const &config = elem.scalable_channel_layout_config;
    if(config.num_layers == 0) {
      out->error("[Section 3.6.2] num_layers SHALL NOT be set to 0.");
    }
    if(config.num_layers > 6) {
      out->error("[Section 3.6.2] num_layers maximum value SHALL be 6. Found %u", config.num_layers);
    }

    uint64_t total_substream_count = 0;
    bool has_layout_15 = false;
    bool has_layout_9 = false;
    for(auto const &layer : config.channel_audio_layer_config) {
      if(layer.loudspeaker_layout == 15) {
        has_layout_15 = true;
      }
      if(layer.loudspeaker_layout == 9) {
        has_layout_9 = true;
      }

      if(layer.substream_count == 0) {
        out->error("[Section 3.6.2] substream_count SHALL NOT be set to 0 for any layer.");
      }

      if(layer.substream_count < layer.coupled_substream_count) {
        out->error(
          "[Section 3.6.2] substream_count (%u) SHALL be greater than or equal to coupled_substream_count (%u) for "
          "all layers.",
          layer.substream_count, layer.coupled_substream_count);
      }

      total_substream_count += layer.substream_count;
    }
    if(has_layout_15 && config.num_layers != 1) {
      out->error(
        "[Section 3.6.2] For an expanded channel layout defined in expanded_loudspeaker_layout, num_layers SHALL be "
        "set to 1.");
    }
    if(has_layout_9 && config.num_layers != 1) {
      out->error("[Section 3.6.2] If loudspeaker_layout is set to Binaural, num_layers SHALL be set to 1.");
    }

    if(total_substream_count != elem.num_substreams) {
      out->error(
        "[Section 3.6.2] The sum of substream_count across all layers (%lu) SHALL be equal to num_substreams of the "
        "Audio Element (%lu).",
        total_substream_count, elem.num_substreams);
    }
  }
}

void validateScalableChannelLayoutGeneration(const IamfState &state, IReport *out)
{
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;
    if(elem.audio_element_type != AUDIO_ELEMENT_CHANNEL_BASED)
      continue;

    auto const &config = elem.scalable_channel_layout_config;
    if(config.num_layers <= 1)
      continue;

    // x: surround channel count, y: LFE channel count, z: height channel count
    int prev_x = -1, prev_y = -1, prev_z = -1;
    for(size_t i = 0; i < config.channel_audio_layer_config.size(); ++i) {
      auto const &layer = config.channel_audio_layer_config[i];
      int x = 0, y = 0, z = 0;
      if(!getXYZChannelCounts(layer.loudspeaker_layout, x, y, z)) {
        continue;
      }

      if(i > 0) {
        if(x < prev_x || y < prev_y || z < prev_z) {
          out->error(
            "[Section 3.6.3] Channel layouts SHALL follow non-decreasing order. Layer %zu has (%d,%d,%d), previous "
            "had (%d,%d,%d)",
            i + 1, x, y, z, prev_x, prev_y, prev_z);
        }
        if(x == prev_x && y == prev_y && z == prev_z) {
          out->error(
            "[Section 3.6.3] Duplicate layers are NOT allowed. Layer %zu has same counts as previous (%d,%d,%d)", i + 1,
            x, y, z);
        }
      }
      prev_x = x;
      prev_y = y;
      prev_z = z;
    }
  }
}

void validateScalableChannelGroupFormat(const IamfState &state, IReport *out)
{
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;
    if(elem.audio_element_type != AUDIO_ELEMENT_CHANNEL_BASED)
      continue;

    auto const &config = elem.scalable_channel_layout_config;

    int prev_layout = -1;
    for(size_t i = 0; i < config.channel_audio_layer_config.size(); ++i) {
      auto const &layer = config.channel_audio_layer_config[i];

      int num_subs = 0, num_coupled = 0;
      if(getExpectedSubstreamCountsForScalableLayer(prev_layout, layer.loudspeaker_layout, num_subs, num_coupled)) {
        if(layer.substream_count != num_subs) {
          out->error(
            "[Section 3.6.3] Layer %zu (layout %u) has substream_count %u, expected %d", i + 1,
            layer.loudspeaker_layout, layer.substream_count, num_subs);
        }
        if(layer.coupled_substream_count != num_coupled) {
          out->error(
            "[Section 3.6.3] Layer %zu (layout %u) has coupled_substream_count %u, expected %d", i + 1,
            layer.loudspeaker_layout, layer.coupled_substream_count, num_coupled);
        }
      }
      prev_layout = layer.loudspeaker_layout;
    }
  }
}

void parseIacb(IReader *br)
{
  br->sym("configurationVersion", 8);
  leb128_read(br); // configOBUs_size

  IamfState state;
  try {
    while(!br->empty()) {
      if(parseIamfObus(br, state) == -1) {
        break;
      }
    }
  } catch(const std::exception &e) {
    fprintf(stderr, "Exception caught during IAMF OBU parsing: %s. Skipping remaining data.\n", e.what());
    auto boxReader = dynamic_cast<BoxReader *>(br);
    if(boxReader) {
      boxReader->br.m_pos = boxReader->br.size * 8;
    }
  }
}
