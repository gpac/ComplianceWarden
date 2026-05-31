#include "iamf_utils.h"

#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"

#include <map>
#include <memory>
#include <set>

#include "bit_reader_utils.h"

namespace
{

void parseIASequenceHeaderOBU(ReaderBits *br, IamfState &state)
{
  state.ia_code = br->sym("ia_code", 32);
  state.primary_profile = br->sym("primary_profile", 8);
  state.additional_profile = br->sym("additional_profile", 8);
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

std::string read_string(ReaderBits *br)
{
  std::string s;
  while(true) {
    char c = br->sym("char", 8);
    if(c == '\0')
      break;
    s += c;
  }
  return s;
}

void parseMixPresentationOBU(ReaderBits *br, MixPresentationInfo &info, uint64_t obuSize)
{
  info.mix_presentation_id = leb128_read(br);
  uint64_t count_label = leb128_read(br);
  for(uint64_t i = 0; i < count_label; i++) {
    read_string(br); // annotations_language
  }
  for(uint64_t i = 0; i < count_label; i++) {
    read_string(br); // localized_presentation_annotations
  }

  uint64_t num_sub_mixes = leb128_read(br);
  for(uint64_t i = 0; i < num_sub_mixes; i++) {
    SubMixInfo sub_mix;
    uint64_t num_audio_elements = leb128_read(br);
    for(uint64_t j = 0; j < num_audio_elements; j++) {
      SubMixAudioElementInfo elem;
      elem.audio_element_id = leb128_read(br);
      for(uint64_t k = 0; k < count_label; k++) {
        read_string(br); // localized_element_annotations
      }

      // Rendering Config
      elem.rendering_config.headphones_rendering_mode = br->sym("headphones_rendering_mode", 2);
      br->sym("reserved", 6);
      uint64_t rendering_config_extension_size = leb128_read(br);
      for(uint64_t k = 0; k < rendering_config_extension_size; k++) {
        br->sym("rendering_config_extension_byte", 8);
      }

      // Element Mix Gain
      static_cast<ParamDefinition &>(elem.element_mix_gain) = parseParamDefinition(br);
      elem.element_mix_gain.default_mix_gain = br->sym("default_mix_gain", 16);

      sub_mix.audio_elements.push_back(elem);
    }

    // Output Mix Gain
    static_cast<ParamDefinition &>(sub_mix.output_mix_gain) = parseParamDefinition(br);
    sub_mix.output_mix_gain.default_mix_gain = br->sym("default_mix_gain", 16);

    uint64_t num_layouts = leb128_read(br);
    for(uint64_t j = 0; j < num_layouts; j++) {
      Layout layout;
      layout.layout_type = br->sym("layout_type", 2);
      if(layout.layout_type == 2) {
        layout.sound_system = br->sym("sound_system", 4);
        br->sym("reserved", 2);
      } else if(layout.layout_type == 3) {
        br->sym("reserved", 6);
      } else {
        br->sym("reserved", 6);
      }

      LoudnessInfo loudness;
      loudness.info_type = br->sym("info_type", 8);
      loudness.integrated_loudness = br->sym("integrated_loudness", 16);
      loudness.digital_peak = br->sym("digital_peak", 16);
      if(loudness.info_type & 1) {
        loudness.true_peak = br->sym("true_peak", 16);
      }
      if(loudness.info_type & 2) {
        uint8_t num_anchored_loudness = br->sym("num_anchored_loudness", 8);
        for(int k = 0; k < num_anchored_loudness; k++) {
          AnchoredLoudness anchor;
          anchor.anchor_element = br->sym("anchor_element", 8);
          anchor.anchored_loudness = br->sym("anchored_loudness", 16);
          loudness.anchored_loudnesses.push_back(anchor);
        }
      }
      if((loudness.info_type & 0b11111100) > 0) {
        uint64_t info_type_size = leb128_read(br);
        for(uint64_t k = 0; k < info_type_size; k++) {
          br->sym("info_type_byte", 8);
        }
      }

      sub_mix.layouts.push_back({ layout, loudness });
    }
    info.sub_mixes.push_back(sub_mix);
  }

  // Mix Presentation Tags
  // As per spec, tags are present if OBU size > size up to end of num_sub_mixes loop.
  if(obuSize > static_cast<uint64_t>(br->count) / 8) {
    uint8_t num_tags = br->sym("num_tags", 8);
    for(int i = 0; i < num_tags; i++) {
      std::string tag_name = read_string(br);
      std::string tag_value = read_string(br);
      info.mix_presentation_tags.push_back({ tag_name, tag_value });
    }
  }
}

bool isValidIso6392Code(const std::string &code)
{
  static const std::set<std::string> kValidCodes = {
    "aar", "abk", "ace", "ach", "ada", "ady", "afa", "afh", "afr", "ain", "aka", "akk", "alb", "ale", "alg", "alt",
    "amh", "ang", "anp", "apa", "ara", "arc", "arg", "arm", "arn", "arp", "art", "arw", "asm", "ast", "ath", "aus",
    "ava", "ave", "awa", "aym", "aze", "bad", "bai", "bak", "bal", "bam", "ban", "baq", "bas", "bat", "bej", "bel",
    "bem", "ben", "ber", "bho", "bih", "bik", "bin", "bis", "bla", "bnt", "bod", "bos", "bra", "bre", "btk", "bua",
    "bug", "bul", "bur", "byn", "cad", "cai", "car", "cat", "cau", "ceb", "cel", "ces", "cha", "chb", "che", "chg",
    "chi", "chk", "chm", "chn", "cho", "chp", "chr", "chu", "chv", "chy", "cmc", "cnr", "cop", "cor", "cos", "cpe",
    "cpf", "cpp", "cre", "crh", "crp", "csb", "cus", "cym", "cze", "dak", "dan", "dar", "day", "del", "den", "deu",
    "dgr", "din", "div", "doi", "dra", "dsb", "dua", "dum", "dut", "dyu", "dzo", "efi", "egy", "eka", "ell", "elx",
    "eng", "enm", "epo", "est", "ewe", "ewo", "fan", "fao", "fas", "fat", "fij", "fil", "fin", "fiu", "fon", "fra",
    "fre", "frm", "fro", "frr", "frs", "fry", "ful", "fur", "gaa", "gay", "gba", "gem", "geo", "ger", "gez", "gil",
    "gla", "gle", "glg", "glv", "gmh", "goh", "gon", "gor", "got", "grb", "grc", "gre", "grn", "gsw", "guj", "gwi",
    "hai", "hat", "hau", "haw", "heb", "her", "hil", "him", "hin", "hit", "hmn", "hmo", "hrv", "hsb", "hun", "hup",
    "hye", "iba", "ibo", "ice", "ido", "iii", "ijo", "iku", "ile", "ilo", "ina", "inc", "ind", "ine", "inh", "ipk",
    "ira", "iro", "isl", "ita", "jav", "jbo", "jpn", "jpr", "jrb", "kaa", "kab", "kac", "kal", "kam", "kan", "kar",
    "kas", "kat", "kau", "kaw", "kaz", "kbd", "kha", "khi", "khm", "kho", "kik", "kin", "kir", "kmb", "kok", "kom",
    "kon", "kor", "kos", "kpe", "krc", "krl", "kro", "kru", "kua", "kum", "kur", "kut", "lad", "lah", "lam", "lao",
    "lat", "lav", "lez", "lim", "lin", "lit", "lol", "loz", "ltz", "lua", "lub", "lug", "lui", "lun", "luo", "lus",
    "mac", "mad", "mag", "mah", "mai", "mak", "mal", "man", "mao", "map", "mar", "mas", "may", "mdf", "mdr", "men",
    "mga", "mic", "min", "mis", "mkd", "mkh", "mlg", "mlt", "mnc", "mni", "mno", "moh", "mon", "mos", "mri", "msa",
    "mul", "mun", "mus", "mwl", "mwr", "mya", "myn", "myv", "nah", "nai", "nap", "nau", "nav", "nbl", "nde", "ndo",
    "nds", "nep", "new", "nia", "nic", "niu", "nld", "nno", "nob", "nog", "non", "nor", "nqo", "nso", "nub", "nwc",
    "nya", "nym", "nyn", "nyo", "nzi", "oci", "oji", "ori", "orm", "osa", "oss", "ota", "oto", "paa", "pag", "pal",
    "pam", "pan", "pap", "pau", "peo", "per", "phi", "phn", "pli", "pol", "pon", "por", "pra", "pro", "pus", "que",
    "raj", "rap", "rar", "roa", "roh", "rom", "ron", "rum", "run", "rup", "rus", "sad", "sag", "sah", "sai", "sal",
    "sam", "san", "sas", "sat", "scn", "sco", "sel", "sem", "sga", "sgn", "shn", "sid", "sin", "sio", "sit", "sla",
    "slk", "slo", "slv", "sma", "sme", "smi", "smj", "smn", "smo", "sms", "sna", "snd", "snk", "sog", "som", "son",
    "sot", "spa", "sqi", "srd", "srn", "srp", "srr", "ssa", "ssw", "suk", "sun", "sus", "sux", "swa", "swe", "syc",
    "syr", "tah", "tai", "tam", "tat", "tel", "tem", "ter", "tet", "tgk", "tgl", "tha", "tib", "tig", "tir", "tiv",
    "tkl", "tlh", "tli", "tmh", "tog", "ton", "tpi", "tsi", "tsn", "tso", "tuk", "tum", "tup", "tur", "tut", "tvl",
    "twi", "tyv", "udm", "uga", "uig", "ukr", "umb", "und", "urd", "uzb", "vai", "ven", "vie", "vol", "vot", "wak",
    "wal", "war", "was", "wel", "wen", "wln", "wol", "xal", "xho", "yao", "yap", "yid", "yor", "ypk", "zap", "zbl",
    "zen", "zgh", "zha", "zho", "znd", "zul", "zun", "zxx", "zza"
  };
  if(code.length() != 3)
    return false;
  if(code >= "qaa" && code <= "qtz")
    return true;
  return kValidCodes.find(code) != kValidCodes.end();
}

const ParamDefinition *findParamDefinition(
  const IamfState &state, uint64_t parameter_id, uint64_t &param_definition_type,
  const AudioElementInfo *&audio_element_ptr)
{
  audio_element_ptr = nullptr;
  for(const auto &ae : state.audioElements) {
    for(size_t i = 0; i < ae.param_definitions.size(); ++i) {
      if(ae.param_definitions[i].parameter_id == parameter_id) {
        param_definition_type = ae.param_definition_types[i];
        audio_element_ptr = &ae;
        return &ae.param_definitions[i];
      }
    }
  }
  for(const auto &mp : state.mixPresentations) {
    for(const auto &sm : mp.sub_mixes) {
      for(const auto &sae : sm.audio_elements) {
        if(sae.element_mix_gain.parameter_id == parameter_id) {
          param_definition_type = PARAMETER_DEFINITION_MIX_GAIN;
          return &sae.element_mix_gain;
        }
      }
      if(sm.output_mix_gain.parameter_id == parameter_id) {
        param_definition_type = PARAMETER_DEFINITION_MIX_GAIN;
        return &sm.output_mix_gain;
      }
    }
  }
  return nullptr;
}

void parseAnimatedParameterData(ReaderBits *br, int animation_type, int bit_width)
{
  if(animation_type == 0) { // STEP
    br->sym("start_point_value", bit_width);
  } else if(animation_type == 1) { // LINEAR
    br->sym("start_point_value", bit_width);
    br->sym("end_point_value", bit_width);
  } else if(animation_type == 2) { // BEZIER
    br->sym("start_point_value", bit_width);
    br->sym("end_point_value", bit_width);
    br->sym("control_point_value", bit_width);
    br->sym("control_point_relative_time", 8);
  }
}

void parseParameterBlockOBU(ReaderBits *br, IamfState &state)
{
  uint64_t parameter_id = leb128_read(br);

  uint64_t param_definition_type = 0;
  const AudioElementInfo *audio_element_ptr = nullptr;
  const ParamDefinition *param_def = findParamDefinition(state, parameter_id, param_definition_type, audio_element_ptr);

  if(!param_def) {
    fprintf(
      stderr, "Warning: Parameter definition not found for parameter_id %llu\n", (unsigned long long)parameter_id);
    return;
  }

  ParameterBlockInfo pb_info;
  pb_info.parameter_id = parameter_id;

  uint8_t param_definition_mode = param_def->param_definition_mode;
  uint64_t duration = param_def->duration;
  uint64_t constant_subblock_duration = param_def->constant_subblock_duration;
  uint64_t num_subblocks = param_def->num_subblocks;

  if(param_definition_mode == 1) {
    duration = leb128_read(br);
    constant_subblock_duration = leb128_read(br);
    if(constant_subblock_duration == 0) {
      num_subblocks = leb128_read(br);
    }
  }

  if(constant_subblock_duration != 0) {
    if(constant_subblock_duration > 0) {
      num_subblocks = (duration + constant_subblock_duration - 1) / constant_subblock_duration;
    }
  }

  pb_info.duration = duration;
  pb_info.constant_subblock_duration = constant_subblock_duration;
  pb_info.num_subblocks = num_subblocks;

  for(uint64_t i = 0; i < num_subblocks; i++) {
    if(param_definition_mode == 1) {
      if(constant_subblock_duration == 0) {
        uint64_t subblock_dur = leb128_read(br);
        pb_info.subblock_durations.push_back(subblock_dur);
      } else {
        if(i < num_subblocks - 1) {
          pb_info.subblock_durations.push_back(constant_subblock_duration);
        } else {
          pb_info.subblock_durations.push_back(duration - (num_subblocks - 1) * constant_subblock_duration);
        }
      }
    } else {
      if(param_def->constant_subblock_duration == 0) {
        if(i < param_def->subblock_durations.size()) {
          pb_info.subblock_durations.push_back(param_def->subblock_durations[i]);
        }
      } else {
        if(i < num_subblocks - 1) {
          pb_info.subblock_durations.push_back(param_def->constant_subblock_duration);
        } else {
          pb_info.subblock_durations.push_back(
            param_def->duration - (num_subblocks - 1) * param_def->constant_subblock_duration);
        }
      }
    }

    if(param_definition_type == PARAMETER_DEFINITION_MIX_GAIN) {
      auto animation_type = leb128_read(br);
      parseAnimatedParameterData(br, animation_type, 16);
    } else if(param_definition_type == PARAMETER_DEFINITION_DEMIXING) {
      br->sym("dmixp_mode", 3);
      br->sym("reserved", 5);
    } else if(param_definition_type == PARAMETER_DEFINITION_RECON_GAIN) {
      if(audio_element_ptr && audio_element_ptr->audio_element_type == AUDIO_ELEMENT_CHANNEL_BASED) {
        const auto &config = audio_element_ptr->scalable_channel_layout_config;
        for(const auto &layer : config.channel_audio_layer_config) {
          if(layer.recon_gain_is_present_flag) {
            uint64_t recon_gain_flags = leb128_read(br);

            int n = (recon_gain_flags < 128) ? 7 : 12;
            for(int j = 0; j < n; j++) {
              if((recon_gain_flags >> j) & 1) {
                br->sym("recon_gain", 8);
              }
            }
          }
        }
      }
    } else {
      uint64_t parameter_data_size = leb128_read(br);
      for(uint64_t j = 0; j < parameter_data_size; j++) {
        br->sym("parameter_data_byte", 8);
      }
    }
  }

  state.parameterBlocks.push_back(pb_info);
}

void checkTicksPerFrame(const ParamDefinition &param, uint64_t codec_config_id, const IamfState &state, IReport *out)
{
  const CodecConfigInfo *matched_config = nullptr;
  for(auto const &config : state.codecConfigs) {
    if(config.codec_config_id == codec_config_id) {
      matched_config = &config;
      break;
    }
  }

  if(matched_config) {
    uint32_t sample_rate = 0;
    if(matched_config->codec_id == FOURCC("Opus")) {
      sample_rate = 48000;
    } else if(matched_config->codec_id == FOURCC("ipcm")) {
      auto pcm_config = dynamic_cast<const LpcmDecoderConfig *>(matched_config->decoder_config.get());
      if(pcm_config) {
        sample_rate = pcm_config->sample_rate;
      }
    }

    if(sample_rate != 0) {
      uint64_t ticks_per_frame_num = param.parameter_rate * matched_config->num_samples_per_frame;
      if(ticks_per_frame_num % sample_rate != 0) {
        out->error(
          "[Section 3.6.1] The parameter rate SHALL be a value such that the number of ticks per frame is an integer. "
          "Found %lu * %lu / %u",
          param.parameter_rate, matched_config->num_samples_per_frame, sample_rate);
      } else if(ticks_per_frame_num / sample_rate == 0) {
        out->error(
          "[Section 3.6.1] The parameter rate SHALL be a value such that the number of ticks per frame is a non-zero "
          "integer. Found 0.");
      }
    }
  }
}

int getAudioElementChannelCount(const AudioElementInfo &elem)
{
  int channels = 0;
  if(elem.audio_element_type == AUDIO_ELEMENT_CHANNEL_BASED) {
    for(const auto &layer : elem.scalable_channel_layout_config.channel_audio_layer_config) {
      channels += layer.substream_count + layer.coupled_substream_count;
    }
  } else if(elem.audio_element_type == AUDIO_ELEMENT_SCENE_BASED) {
    if(elem.ambisonics_config.ambisonics_mode == 0) {
      channels = elem.ambisonics_config.mono_config.substream_count;
    } else if(elem.ambisonics_config.ambisonics_mode == 1) {
      channels = elem.ambisonics_config.projection_config.substream_count +
        elem.ambisonics_config.projection_config.coupled_substream_count;
    }
  }
  return channels;
}

} // namespace

int64_t parseIamfObus(IReader *br, IamfState &state)
{
  br->sym("obu", 0); // virtual OBU separator
  auto obu_type = br->sym("obu_type", 5);
  state.obu_redundant_copy = br->sym("obu_redundant_copy", 1);
  state.obu_trimming_status_flag = br->sym("obu_trimming_status_flag", 1);
  auto obu_extension_flag = br->sym("obu_extension_flag", 1);

  bool is_audio_frame =
    (obu_type == OBU_IA_Audio_Frame) || (obu_type >= OBU_IA_Audio_Frame_ID0 && obu_type <= OBU_IA_Audio_Frame_ID17);
  if(!is_audio_frame && state.obu_trimming_status_flag != 0) {
    state.has_invalid_trimming_flag = true;
  }

  ReaderBits brSize(br);
  uint64_t obuSize = leb128_read(&brSize);
  // Number of bytes used to encode the OBU size. Since leb128_read reads full bytes (8 bits at a time),
  // brSize.count will always be a multiple of 8.
  int leb128_bytes = brSize.count / 8;

  // Total OBU size = 1 byte (header) + bytes used for obu_size field + payload size.
  uint64_t total_obu_size = 1 + leb128_bytes + obuSize;
  if(total_obu_size > state.max_obu_size_seen) {
    state.max_obu_size_seen = total_obu_size;
  }
  auto brBits = std::make_unique<ReaderBits>(br);

  uint64_t num_samples_to_trim_at_end = 0;
  uint64_t num_samples_to_trim_at_start = 0;
  if(state.obu_trimming_status_flag) {
    num_samples_to_trim_at_end = leb128_read(brBits.get());
    num_samples_to_trim_at_start = leb128_read(brBits.get());
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

    std::unique_ptr<DecoderConfig> decoder_config = nullptr;
    if(codec_id == FOURCC("Opus")) {
      auto opus_config = std::make_unique<OpusDecoderConfig>();
      opus_config->version = brBits->sym("version", 8);
      opus_config->output_channel_count = brBits->sym("output_channel_count", 8);
      opus_config->pre_skip = brBits->sym("pre_skip", 16);
      opus_config->input_sample_rate = brBits->sym("input_sample_rate", 32);
      opus_config->output_gain = brBits->sym("output_gain", 16);
      opus_config->channel_mapping_family = brBits->sym("channel_mapping_family", 8);
      decoder_config = std::move(opus_config);
    } else if(codec_id == FOURCC("ipcm")) {
      auto pcm_config = std::make_unique<LpcmDecoderConfig>();
      pcm_config->sample_format_flags = brBits->sym("sample_format_flags", 8);
      pcm_config->sample_size = brBits->sym("sample_size", 8);
      pcm_config->sample_rate = brBits->sym("sample_rate", 32);
      decoder_config = std::move(pcm_config);
    }

    state.codecConfigs.push_back(
      { codec_config_id, codec_id, num_samples_per_frame, audio_roll_distance, std::move(decoder_config) });

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

  case OBU_IA_Mix_Presentation: {
    br->sym("mix_presentation", 0);
    MixPresentationInfo info;
    parseMixPresentationOBU(brBits.get(), info, obuSize);
    state.mixPresentations.push_back(info);
    br->sym("/mix_presentation", 0);
    break;
  }

  case OBU_IA_Parameter_Block:
    br->sym("parameter_block", 0);
    parseParameterBlockOBU(brBits.get(), state);
    br->sym("/parameter_block", 0);
    break;

  case OBU_IA_Temporal_Delimiter:
    br->sym("temporal_delimiter", 0);
    br->sym("/temporal_delimiter", 0);
    break;

  case OBU_IA_Audio_Frame:
  case OBU_IA_Audio_Frame_ID0:
  case OBU_IA_Audio_Frame_ID1:
  case OBU_IA_Audio_Frame_ID2:
  case OBU_IA_Audio_Frame_ID3:
  case OBU_IA_Audio_Frame_ID4:
  case OBU_IA_Audio_Frame_ID5:
  case OBU_IA_Audio_Frame_ID6:
  case OBU_IA_Audio_Frame_ID7:
  case OBU_IA_Audio_Frame_ID8:
  case OBU_IA_Audio_Frame_ID9:
  case OBU_IA_Audio_Frame_ID10:
  case OBU_IA_Audio_Frame_ID11:
  case OBU_IA_Audio_Frame_ID12:
  case OBU_IA_Audio_Frame_ID13:
  case OBU_IA_Audio_Frame_ID14:
  case OBU_IA_Audio_Frame_ID15:
  case OBU_IA_Audio_Frame_ID16:
  case OBU_IA_Audio_Frame_ID17: {
    br->sym("audio_frame", 0);

    bool audio_substream_id_in_bitstream = (obu_type == OBU_IA_Audio_Frame);
    uint64_t substream_id = 0;
    if(audio_substream_id_in_bitstream) {
      substream_id = leb128_read(brBits.get());
    } else {
      substream_id = obu_type - OBU_IA_Audio_Frame_ID0;
    }

    state.audioFrames.push_back(
      { static_cast<uint64_t>(obu_type), substream_id, num_samples_to_trim_at_start, num_samples_to_trim_at_end });

    // The rest of the OBU is the codec-specific audio_frame data. We do not need to
    // read it here as it is not required for structural validation, and the common
    // code at the bottom of parseIamfObus will automatically skip any unread bytes.

    br->sym("/audio_frame", 0);
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

  state.obuSequence.push_back({ obu_type, state.obu_redundant_copy });
  return obu_type;
}

void validateSequenceHeaderTrimming(const IamfState &state, IReport *out)
{
  if(state.seenSequenceHeader) {
    if(state.seq_hdr_obu_trimming_status_flag != 0) {
      out->error(
        "[Section 3.2] obu_trimming_status_flag SHALL be 0 for IA Sequence Header OBU, found %d",
        state.seq_hdr_obu_trimming_status_flag);
    }
  }
}

void validateObuTrimming(const IamfState &state, IReport *out)
{
  if(state.has_invalid_trimming_flag) {
    out->error("[Section 3.2] obu_trimming_status_flag SHALL be set to 0 for all OBUs except Audio Frame OBUs.");
  }

  // Group frames by substream_id
  std::map<uint64_t, std::vector<AudioFrameInfo>> substream_frames;
  for(auto const &af : state.audioFrames) {
    substream_frames[af.substream_id].push_back(af);
  }

  // Pre-compute map from substream_id to num_samples_per_frame
  std::map<uint64_t, uint64_t> substream_to_frame_size;
  for(auto const &elem : state.audioElements) {
    uint64_t frame_size = 0;
    for(auto const &config : state.codecConfigs) {
      if(config.codec_config_id == elem.codec_config_id) {
        frame_size = config.num_samples_per_frame;
        break;
      }
    }
    if(frame_size == 0)
      continue;

    for(auto id : elem.audio_substream_ids) {
      substream_to_frame_size[id] = frame_size;
    }
  }

  for(auto const &p : substream_frames) {
    uint64_t substream_id = p.first;
    auto const &frames = p.second;

    auto it = substream_to_frame_size.find(substream_id);
    if(it == substream_to_frame_size.end())
      continue;
    uint64_t num_samples_per_frame = it->second;

    bool seen_partial_or_no_trim = false;
    for(size_t i = 0; i < frames.size(); ++i) {
      uint64_t trim = frames[i].num_samples_to_trim_at_start;

      if(seen_partial_or_no_trim) {
        if(trim != 0) {
          out->error(
            "[Section 3.2] Audio Frame OBU for substream %lu has non-zero num_samples_to_trim_at_start (%lu) after a "
            "frame that was not fully trimmed.",
            substream_id, trim);
        }
      } else {
        if(trim < num_samples_per_frame) {
          seen_partial_or_no_trim = true;
        }
      }
    }
  }
}

void validateObuMaxSize(const IamfState &state, IReport *out)
{
  if(state.max_obu_size_seen > 2097152) {
    out->error(
      "[Section 4] The maximum size of an OBU SHALL be limited to 2MB (2097152 bytes), found %lu",
      state.max_obu_size_seen);
  }
}

void validateCommonProfileRestrictions(const IamfState &state, IReport *out)

{
  // 1. OBU max size
  validateObuMaxSize(state, out);

  // 2. Unique Codec Config
  std::set<uint64_t> unique_codec_config_ids;
  for(auto const &config : state.codecConfigs) {
    unique_codec_config_ids.insert(config.codec_config_id);
  }
  if(unique_codec_config_ids.size() > 1) {
    out->error(
      "[Section 4] There SHALL be only one unique Codec Config OBU. Found %zu unique IDs.",
      unique_codec_config_ids.size());
  }

  // 3. Mix Presentation restrictions
  for(auto const &mix : state.mixPresentations) {
    if(mix.sub_mixes.size() > 1) {
      out->warning("[Section 4] num_sub_mixes SHOULD be set to 1. Found %zu", mix.sub_mixes.size());
    }
    for(auto const &sub_mix : mix.sub_mixes) {
      if(sub_mix.audio_elements.size() > 28) {
        out->warning(
          "[Section 4] num_audio_elements SHOULD be set to at most 28. Found %zu in a sub-mix",
          sub_mix.audio_elements.size());
      }
    }
  }

  // 4. Scalable Channel Layout num_layers=1
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;
    if(elem.audio_element_type != AUDIO_ELEMENT_CHANNEL_BASED)
      continue;

    auto const &config = elem.scalable_channel_layout_config;
    if(config.num_layers == 1) {
      if(!config.channel_audio_layer_config.empty()) {
        auto const &layer = config.channel_audio_layer_config[0];
        if(layer.output_gain_is_present_flag != 0) {
          out->error("[Section 4] When num_layers = 1, output_gain_is_present_flag SHALL be set to 0.");
        }
        if(layer.recon_gain_is_present_flag != 0) {
          out->error("[Section 4] When num_layers = 1, recon_gain_is_present_flag SHALL be set to 0.");
        }
      }
    }
  }
}

void validateProfileRestrictions(const IamfState &state, IReport *out, int target_profile)
{
  if(state.audioElements.empty()) {
    return;
  }

  std::map<uint64_t, const AudioElementInfo *> id_to_ae;
  for(const auto &ae : state.audioElements) {
    if(id_to_ae.find(ae.audio_element_id) == id_to_ae.end()) {
      id_to_ae[ae.audio_element_id] = &ae;
    }
  }

  struct Counts {
    int num_scene_based_elements = 0;
    int num_multi_layer_channel_elements = 0;
    int total_channels = 0;
    int num_valid_elements = 0;
  };

  auto getCounts = [&](const std::set<uint64_t> &ids) {
    Counts counts;
    for(auto id : ids) {
      auto it = id_to_ae.find(id);
      if(it == id_to_ae.end() || it->second->ignored) {
        continue;
      }
      const auto &elem = *it->second;
      if(elem.audio_element_type == AUDIO_ELEMENT_SCENE_BASED) {
        counts.num_scene_based_elements++;
      } else if(elem.audio_element_type == AUDIO_ELEMENT_CHANNEL_BASED) {
        if(elem.scalable_channel_layout_config.num_layers > 1) {
          counts.num_multi_layer_channel_elements++;
        }
      }
      counts.total_channels += getAudioElementChannelCount(elem);
      counts.num_valid_elements++;
    }
    return counts;
  };

  // --- Simple Profile checks ---
  bool check_entire_sequence_simple = (target_profile == 0 && state.additional_profile == 0);
  bool check_at_least_one_mix_simple = (target_profile == 0 && state.primary_profile == 0);

  if(check_entire_sequence_simple || check_at_least_one_mix_simple) {
    out->covered();

    if(check_entire_sequence_simple) {
      if(state.audioElements.size() > 1) {
        out->error(
          "[Section 4.1] For Simple Profile, only one unique Audio Element OBU is allowed. Found %zu",
          state.audioElements.size());
      }

      const auto &elem = state.audioElements[0];
      if(elem.ignored) {
        out->warning(
          "[Section 4.1] Ignored Audio Element %lu with unknown type %u.", elem.audio_element_id,
          elem.audio_element_type);
      } else {
        int channels = getAudioElementChannelCount(elem);
        if(channels > 16) {
          out->error(
            "[Section 4.1] For Simple Profile, Audio Element %lu has %d channels, which exceeds the limit of 16.",
            elem.audio_element_id, channels);
        }
      }
    }

    if(check_at_least_one_mix_simple) {
      bool found_compliant_mix = false;
      for(const auto &mix : state.mixPresentations) {
        std::set<uint64_t> ae_ids;
        for(const auto &sub_mix : mix.sub_mixes) {
          for(const auto &sub_elem : sub_mix.audio_elements) {
            ae_ids.insert(sub_elem.audio_element_id);
          }
        }
        auto counts = getCounts(ae_ids);
        if(ae_ids.size() == 1 && counts.num_valid_elements == 1 && counts.total_channels <= 16) {
          found_compliant_mix = true;
          break;
        }
      }

      if(!found_compliant_mix) {
        out->error("[Section 4.1] For Simple Profile, no Mix Presentation complies with Simple Profile constraints.");
      }
    }
  }

  // --- Base Profile checks ---
  bool check_entire_sequence_base = (target_profile == 1 && state.additional_profile == 1);
  bool check_at_least_one_mix_base = (target_profile == 1 && state.primary_profile == 1);

  if(check_entire_sequence_base || check_at_least_one_mix_base) {
    out->covered();

    if(check_entire_sequence_base) {
      if(state.audioElements.size() > 2) {
        out->error(
          "[Section 4.2] For Base Profile, at most two unique Audio Element OBUs are allowed. Found %zu",
          state.audioElements.size());
      }

      std::set<uint64_t> all_ae_ids;
      for(const auto &ae : state.audioElements) {
        all_ae_ids.insert(ae.audio_element_id);
      }

      auto counts = getCounts(all_ae_ids);

      if(counts.num_scene_based_elements > 1) {
        out->error(
          "[Section 4.2] For Base Profile, at most one Scene-based Audio Element is allowed. Found %d",
          counts.num_scene_based_elements);
      }
      if(counts.num_multi_layer_channel_elements > 1) {
        out->error(
          "[Section 4.2] For Base Profile, at most one Channel-based Audio Element having num_layers > 1 "
          "is allowed. Found %d",
          counts.num_multi_layer_channel_elements);
      }
    }

    if(check_entire_sequence_base || check_at_least_one_mix_base) {
      bool found_compliant_mix = false;
      for(const auto &mix : state.mixPresentations) {
        std::set<uint64_t> mix_ids;
        for(const auto &sub_mix : mix.sub_mixes) {
          for(const auto &sub_elem : sub_mix.audio_elements) {
            mix_ids.insert(sub_elem.audio_element_id);
          }
        }
        auto counts = getCounts(mix_ids);

        if(check_entire_sequence_base && counts.total_channels > 18) {
          out->error(
            "[Section 4.2] For Base Profile, the sum of channels across all Audio Elements in Mix Presentation %lu "
            "before mixing exceeds 18. Found %d",
            mix.mix_presentation_id, counts.total_channels);
        }

        if(check_at_least_one_mix_base && !found_compliant_mix) {
          if(
            mix_ids.size() <= 2 && counts.num_scene_based_elements <= 1 &&
            counts.num_multi_layer_channel_elements <= 1 && counts.total_channels <= 18) {
            found_compliant_mix = true;
          }
        }
      }

      if(check_at_least_one_mix_base && !found_compliant_mix) {
        out->error("[Section 4.2] For Base Profile, no Mix Presentation complies with Base Profile constraints.");
      }
    }
  }

  // --- Base-Enhanced Profile checks ---
  bool check_entire_sequence_enhanced =
    (target_profile == 2 && state.additional_profile == 2 && state.primary_profile <= 2);
  bool check_at_least_one_mix_enhanced = (target_profile == 2 && state.primary_profile == 2);

  if(check_entire_sequence_enhanced || check_at_least_one_mix_enhanced) {
    out->covered();

    if(check_entire_sequence_enhanced) {
      std::set<uint64_t> all_ae_ids;
      for(const auto &ae : state.audioElements) {
        all_ae_ids.insert(ae.audio_element_id);
      }

      auto counts = getCounts(all_ae_ids);

      if(counts.total_channels > 28) {
        out->error(
          "[Section 4.3] For Base-Enhanced Profile, at most 28 channels in total across all Audio Elements in "
          "the IA Sequence are allowed. Found %d",
          counts.total_channels);
      }
    }

    if(check_entire_sequence_enhanced || check_at_least_one_mix_enhanced) {
      bool found_compliant_mix = false;
      for(const auto &mix : state.mixPresentations) {
        std::set<uint64_t> mix_ids;
        for(const auto &sub_mix : mix.sub_mixes) {
          for(const auto &sub_elem : sub_mix.audio_elements) {
            mix_ids.insert(sub_elem.audio_element_id);
          }
        }
        auto counts = getCounts(mix_ids);

        if(check_entire_sequence_enhanced && counts.total_channels > 28) {
          out->error(
            "[Section 4.3] For Base-Enhanced Profile, the sum of channels across all Audio Elements in "
            "Mix Presentation %lu before mixing exceeds 28. Found %d",
            mix.mix_presentation_id, counts.total_channels);
        }

        if(check_at_least_one_mix_enhanced && !found_compliant_mix) {
          if(mix_ids.size() <= 28 && counts.total_channels <= 28) {
            found_compliant_mix = true;
          }
        }
      }

      if(check_at_least_one_mix_enhanced && !found_compliant_mix) {
        out->error(
          "[Section 4.3] For Base-Enhanced Profile, no Mix Presentation complies with Base-Enhanced Profile "
          "constraints.");
      }
    }
  }
}

bool validateProfiles(const IamfState &state, IReport *out)
{
  if(state.primary_profile > 2) {
    if(out) {
      out->error("Unsupported primary profile %d", state.primary_profile);
    }
    return false;
  }
  if(state.additional_profile > 2) {
    if(out) {
      out->warning(
        "Unsupported additional profile %d. Compliance will be evaluated against the primary profile.",
        state.additional_profile);
    }
  }
  return true;
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

      checkTicksPerFrame(param, elem.codec_config_id, state, out);

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

  for(auto const &mix : state.mixPresentations) {
    for(auto const &sub_mix : mix.sub_mixes) {
      for(auto const &elem : sub_mix.audio_elements) {
        uint64_t codec_config_id = 0;
        for(auto const &ae : state.audioElements) {
          if(ae.audio_element_id == elem.audio_element_id) {
            codec_config_id = ae.codec_config_id;
            break;
          }
        }
        checkTicksPerFrame(elem.element_mix_gain, codec_config_id, state, out);
      }

      if(!sub_mix.audio_elements.empty()) {
        uint64_t first_ae_id = sub_mix.audio_elements[0].audio_element_id;
        uint64_t codec_config_id = 0;
        for(auto const &ae : state.audioElements) {
          if(ae.audio_element_id == first_ae_id) {
            codec_config_id = ae.codec_config_id;
            break;
          }
        }
        checkTicksPerFrame(sub_mix.output_mix_gain, codec_config_id, state, out);
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

void validateAmbisonicsConfig(const IamfState &state, IReport *out)
{
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;
    if(elem.audio_element_type != AUDIO_ELEMENT_SCENE_BASED)
      continue;

    auto const &config = elem.ambisonics_config;

    if(config.ambisonics_mode != 0 && config.ambisonics_mode != 1) {
      out->error("[Section 3.6.4] ambisonics_mode SHALL be 0 or 1. Found %lu", config.ambisonics_mode);
      continue;
    }

    uint8_t output_channel_count = 0;
    uint8_t substream_count = 0;
    uint8_t coupled_substream_count = 0;

    if(config.ambisonics_mode == 0) {
      output_channel_count = config.mono_config.output_channel_count;
      substream_count = config.mono_config.substream_count;
    } else if(config.ambisonics_mode == 1) {
      output_channel_count = config.projection_config.output_channel_count;
      substream_count = config.projection_config.substream_count;
      coupled_substream_count = config.projection_config.coupled_substream_count;

      if(coupled_substream_count > substream_count) {
        out->error(
          "[Section 3.6.4] coupled_substream_count (%u) SHALL be less than or equal to substream_count (%u).",
          coupled_substream_count, substream_count);
      }
    }

    // Check output_channel_count = (1 + n)^2
    bool valid_channels = false;
    for(int n = 0; n <= 14; ++n) {
      if(output_channel_count == (1 + n) * (1 + n)) {
        valid_channels = true;
        break;
      }
    }
    if(!valid_channels) {
      out->error(
        "[Section 3.6.4] output_channel_count (%u) SHALL be (1 + n)^2 for n = 0, 1, 2, ..., 14.", output_channel_count);
    }

    if(substream_count != elem.num_substreams) {
      out->error(
        "[Section 3.6.4] substream_count (%u) SHALL be the same as num_substreams (%lu) in this OBU.", substream_count,
        elem.num_substreams);
    }
  }
}

void validateMixPresentation(const IamfState &state, IReport *out)
{
  std::set<uint64_t> unique_mix_ids;
  for(auto const &mix : state.mixPresentations) {
    if(!unique_mix_ids.insert(mix.mix_presentation_id).second) {
      out->error("[Section 3.7] mix_presentation_id SHALL be unique within an IA Sequence.");
    }

    if(mix.sub_mixes.empty()) {
      out->error("[Section 3.7] num_sub_mixes SHALL NOT be set to 0.");
    }

    std::set<uint64_t> ae_ids_in_mix;
    for(auto const &sub_mix : mix.sub_mixes) {
      if(sub_mix.audio_elements.empty()) {
        out->error("[Section 3.7] num_audio_elements SHALL NOT be set to 0.");
      }

      for(auto const &elem : sub_mix.audio_elements) {
        if(!ae_ids_in_mix.insert(elem.audio_element_id).second) {
          out->error(
            "[Section 3.7] There SHALL be no duplicate values of audio_element_id within one Mix Presentation.");
        }
        if(elem.rendering_config.headphones_rendering_mode >= 2) {
          out->error(
            "[Section 3.7.3] headphones_rendering_mode must be 0 or 1, found %d",
            elem.rendering_config.headphones_rendering_mode);
        }
      }

      bool has_stereo_loudness = false;
      for(auto const &p : sub_mix.layouts) {
        auto const &layout = p.first;
        if(layout.layout_type == 2 && layout.sound_system == 0) {
          has_stereo_loudness = true;
          break;
        }
      }
      if(!has_stereo_loudness) {
        out->error("[Section 3.7] Each sub-mix SHALL include loudness for Stereo.");
      }
    }
  }
}

void validateMixPresentationLoudness(const IamfState &state, IReport *out)
{
  for(auto const &mix : state.mixPresentations) {
    for(auto const &sub_mix : mix.sub_mixes) {
      for(auto const &p : sub_mix.layouts) {
        auto const &loudness = p.second;
        std::set<uint8_t> anchor_elements;
        for(auto const &anchor : loudness.anchored_loudnesses) {
          if(!anchor_elements.insert(anchor.anchor_element).second) {
            out->error("[Section 3.7.4] There SHALL be no duplicate values of anchor_element within one LoudnessInfo.");
          }
        }
      }
    }
  }
}

void validateMixPresentationTags(const IamfState &state, IReport *out)
{
  for(auto const &mix : state.mixPresentations) {
    int content_language_count = 0;
    for(auto const &tag : mix.mix_presentation_tags) {
      if(tag.first == "content_language") {
        content_language_count++;
        if(!isValidIso6392Code(tag.second)) {
          out->error(
            "[Section 3.7.5] content_language tag value SHALL conform to ISO-639-2-Codes (3-letter code), found '%s'",
            tag.second.c_str());
        }
      }
    }
    if(content_language_count > 1) {
      out->error("[Section 3.7.5] There SHALL be at most one instance of content_language tag.");
    }
  }
}

void validateParameterBlocks(const IamfState &state, IReport *out)
{
  for(auto const &pb : state.parameterBlocks) {
    if(pb.duration == 0) {
      out->error("[Section 3.8] duration SHALL NOT be set to 0.");
    }

    uint64_t sum_durations = 0;
    for(auto dur : pb.subblock_durations) {
      if(dur == 0) {
        out->error("[Section 3.8] subblock_duration SHALL NOT be set to 0.");
      }
      sum_durations += dur;
    }

    if(sum_durations != pb.duration) {
      out->error(
        "[Section 3.8] The summation of all subblock_duration (%lu) SHALL be equal to duration (%lu).", sum_durations,
        pb.duration);
    }
  }
}

void validateAudioFrames(const IamfState &state, IReport *out)
{
  std::set<uint64_t> all_substream_ids;
  for(auto const &elem : state.audioElements) {
    if(elem.ignored)
      continue;
    for(auto id : elem.audio_substream_ids) {
      if(!all_substream_ids.insert(id).second) {
        out->error(
          "[Section 3.9] There SHALL be exactly one Audio Element OBU with a given audio_substream_id in a set of "
          "Descriptors. Duplicate id %lu",
          id);
      }
    }
  }

  for(auto const &af : state.audioFrames) {
    if(af.obu_type == OBU_IA_Audio_Frame) {
      if(af.substream_id <= 17) {
        out->error(
          "[Section 3.9] When obu_type = OBU_IA_Audio_Frame, explicit_audio_substream_id SHALL be greater than 17. "
          "Found %lu",
          af.substream_id);
      }
    }

    bool valid_id = false;
    for(auto const &elem : state.audioElements) {
      for(auto id : elem.audio_substream_ids) {
        if(id == af.substream_id) {
          valid_id = true;
          break;
        }
      }
      if(valid_id)
        break;
    }
    if(!valid_id) {
      out->error("[Section 3.9] AudioFrameOBU refers to an unknown audio_substream_id %lu", af.substream_id);
    }
  }
}

void validateOpusSpecific(const IamfState &state, IReport *out)
{
  for(auto const &config : state.codecConfigs) {
    if(config.codec_id == FOURCC("Opus")) {
      auto opus_config = dynamic_cast<const OpusDecoderConfig *>(config.decoder_config.get());
      if(!opus_config) {
        out->error("[Section 3.11.1] DecoderConfig is missing or not OpusDecoderConfig for Opus codec");
        continue;
      }

      if(opus_config->version != 1) {
        out->error("[Section 3.11.1] Version SHALL be 1 for Opus DecoderConfig, found %d", opus_config->version);
      }
      if(opus_config->output_channel_count != 2) {
        out->error(
          "[Section 3.11.1] Output Channel Count SHALL be set to 2 for Opus, found %d",
          opus_config->output_channel_count);
      }
      if(opus_config->output_gain != 0) {
        out->error("[Section 3.11.1] Output Gain SHALL be set to 0 dB for Opus, found %d", opus_config->output_gain);
      }
      if(opus_config->channel_mapping_family != 0) {
        out->error(
          "[Section 3.11.1] Channel Mapping Family SHALL be set to 0 for Opus, found %d",
          opus_config->channel_mapping_family);
      }

      std::set<uint64_t> substream_ids;
      for(auto const &elem : state.audioElements) {
        if(elem.codec_config_id == config.codec_config_id) {
          for(auto id : elem.audio_substream_ids) {
            substream_ids.insert(id);
          }
        }
      }

      bool has_audio_frames = false;
      for(auto const &frame : state.audioFrames) {
        if(substream_ids.find(frame.substream_id) != substream_ids.end()) {
          has_audio_frames = true;
          break;
        }
      }

      // Section 3.11.1: "Opus : All coded Audio Substreams referred to by all Audio Elements with this codec
      // configuration must comply with the [RFC-6716] specification." Since Opus encoders always introduce a non-zero
      // lookahead, pre_skip must not be 0.
      if(has_audio_frames && opus_config->pre_skip == 0) {
        out->error(
          "[Section 3.11.1] Opus streams must have a non-zero pre_skip (found 0) to account for encoder delay.");
      }

      for(auto substream_id : substream_ids) {
        uint64_t total_trimmed = 0;
        uint64_t num_samples_per_frame = config.num_samples_per_frame;
        for(auto const &frame : state.audioFrames) {
          if(frame.substream_id == substream_id) {
            total_trimmed += frame.num_samples_to_trim_at_start;
            if(frame.num_samples_to_trim_at_start < num_samples_per_frame) {
              break;
            }
          }
        }
        if(opus_config->pre_skip != total_trimmed) {
          out->error(
            "[Section 3.11.1] Pre-skip (%u) SHALL be the same as the total number of audio samples to be trimmed at "
            "the start of coded Audio Substream %lu (%lu)",
            opus_config->pre_skip, substream_id, total_trimmed);
        }
      }
    }
  }
}

void validateLpcmSpecific(const IamfState &state, IReport *out)
{
  for(auto const &config : state.codecConfigs) {
    if(config.codec_id == FOURCC("ipcm")) {
      auto pcm_config = dynamic_cast<const LpcmDecoderConfig *>(config.decoder_config.get());
      if(!pcm_config) {
        out->error("[Section 3.11.4] DecoderConfig is missing or not LpcmDecoderConfig for ipcm codec");
        continue;
      }

      if(pcm_config->sample_format_flags != 0 && pcm_config->sample_format_flags != 1) {
        out->error("[Section 3.11.4] sample_format_flags SHALL be 0 or 1, found %d", pcm_config->sample_format_flags);
      }
      if(pcm_config->sample_size != 16 && pcm_config->sample_size != 24 && pcm_config->sample_size != 32) {
        out->error("[Section 3.11.4] sample_size SHALL be 16, 24, or 32, found %d", pcm_config->sample_size);
      }
      if(
        pcm_config->sample_rate != 44100 && pcm_config->sample_rate != 16000 && pcm_config->sample_rate != 32000 &&
        pcm_config->sample_rate != 48000 && pcm_config->sample_rate != 96000) {
        out->error(
          "[Section 3.11.4] sample_rate SHALL be 44100, 16000, 32000, 48000, or 96000, found %u",
          pcm_config->sample_rate);
      }
    }
  }
}

void validateSubstreamTrimmingConsistency(const IamfState &state, IReport *out)
{
  // Group frames by substream_id
  std::map<uint64_t, std::vector<AudioFrameInfo>> substream_frames;
  for(auto const &af : state.audioFrames) {
    substream_frames[af.substream_id].push_back(af);
  }

  if(substream_frames.empty())
    return;

  // Use the first substream as reference
  auto const &ref_frames = substream_frames.begin()->second;
  uint64_t ref_substream_id = substream_frames.begin()->first;

  for(auto const &p : substream_frames) {
    if(p.first == ref_substream_id)
      continue;
    auto const &frames = p.second;

    if(frames.size() != ref_frames.size()) {
      out->error(
        "[Section 4] Substream %lu has %lu frames, expected %lu (matching substream %lu)", p.first, frames.size(),
        ref_frames.size(), ref_substream_id);
      continue;
    }

    for(size_t i = 0; i < frames.size(); ++i) {
      if(frames[i].num_samples_to_trim_at_start != ref_frames[i].num_samples_to_trim_at_start) {
        out->error(
          "[Section 4] Substream %lu frame %zu has start trim %lu, expected %lu (matching substream %lu)", p.first, i,
          frames[i].num_samples_to_trim_at_start, ref_frames[i].num_samples_to_trim_at_start, ref_substream_id);
      }
      if(frames[i].num_samples_to_trim_at_end != ref_frames[i].num_samples_to_trim_at_end) {
        out->error(
          "[Section 4] Substream %lu frame %zu has end trim %lu, expected %lu (matching substream %lu)", p.first, i,
          frames[i].num_samples_to_trim_at_end, ref_frames[i].num_samples_to_trim_at_end, ref_substream_id);
      }
    }
  }
}

void validateDescriptorObusOrder(const IamfState &state, IReport *out)
{
  if(state.obuSequence.empty()) {
    return;
  }

  auto getDescriptorName = [](int order) {
    switch(order) {
    case -1:
      return "start of sequence";
    case 0:
      return "IA Sequence Header OBU";
    case 1:
      return "Codec Config OBU";
    case 2:
      return "Audio Element OBU";
    case 3:
      return "Mix Presentation OBU";
    default:
      return "Unknown OBU";
    }
  };

  int last_descriptor_order = -1;
  bool in_data = false;
  bool is_first_sequence = true;
  bool current_sequence_requires_full_set = true;

  for(const auto &obu : state.obuSequence) {
    int64_t obu_type = obu.type;

    int current_descriptor_order = -1;
    switch(obu_type) {
    case OBU_IA_Sequence_Header:
      current_descriptor_order = 0;
      break;
    case OBU_IA_Codec_Config:
      current_descriptor_order = 1;
      break;
    case OBU_IA_Audio_Element:
      current_descriptor_order = 2;
      break;
    case OBU_IA_Mix_Presentation:
      current_descriptor_order = 3;
      break;
    default:
      break;
    }

    if(current_descriptor_order >= 0) {
      // Encountered a known Descriptor OBU or Reserved OBUs.
      // Allow Reserved OBUs because the spec does not predetermine if they are Descriptors or IA Data OBUs.
      if(in_data && obu_type != OBU_IA_Sequence_Header) {
        out->error(
          "[Section 5.1.1] Descriptor OBUs must follow IA Sequence Header. Found OBU type %ld after data OBUs.",
          obu_type);
      }

      if(!in_data && obu_type != OBU_IA_Sequence_Header) {
        if(current_descriptor_order < last_descriptor_order) {
          out->error(
            "[Section 5.1.1] Descriptor OBUs out of order. Found %s after %s",
            getDescriptorName(current_descriptor_order), getDescriptorName(last_descriptor_order));
        } else if(current_descriptor_order > last_descriptor_order + 1) {
          out->error(
            "[Section 5.1.1] Descriptor OBUs skipped. Found %s after %s", getDescriptorName(current_descriptor_order),
            getDescriptorName(last_descriptor_order));
        }
      }

      if(obu_type == OBU_IA_Sequence_Header) {
        // Allow redundant IA Sequence Header inserted before descriptors.
        if(
          current_sequence_requires_full_set && last_descriptor_order != 3 && last_descriptor_order != -1 &&
          last_descriptor_order != 0) {
          out->error(
            "[Section 5.1.1] Missing required descriptor OBUs. Last seen %s before IA Sequence Header OBU",
            getDescriptorName(last_descriptor_order));
        }
        current_sequence_requires_full_set = is_first_sequence || (obu.obu_redundant_copy == 0);
        is_first_sequence = false;
      }

      in_data = false;
      last_descriptor_order = current_descriptor_order;
    } else if(obu_type >= OBU_IA_Parameter_Block && obu_type <= OBU_IA_Audio_Frame_ID17) {
      // Encountered a known IA Data OBU
      if(!in_data) {
        if(current_sequence_requires_full_set && last_descriptor_order != 3) {
          out->error(
            "[Section 5.1.1] Missing required descriptor OBUs before data OBUs. Last seen %s before OBU type %ld",
            getDescriptorName(last_descriptor_order), obu_type);
        }
      }
      in_data = true;
    }
  }

  // Handle case where bitstream ends without any data OBUs
  if(!in_data) {
    if(current_sequence_requires_full_set && last_descriptor_order != 3) {
      out->error(
        "[Section 5.1.1] Missing required descriptor OBUs. Reached end of bitstream. Last seen %s",
        getDescriptorName(last_descriptor_order));
    }
  }
}

void validateParameterSubstreamConsistency(const IamfState &state, IReport *out)
{
  if(state.codecConfigs.empty())
    return;
  const auto &config = state.codecConfigs[0];
  uint32_t audio_sample_rate = 0;
  if(config.codec_id == FOURCC("Opus")) {
    audio_sample_rate = 48000;
  } else if(config.codec_id == FOURCC("ipcm")) {
    auto pcm_config = dynamic_cast<const LpcmDecoderConfig *>(config.decoder_config.get());
    if(pcm_config) {
      audio_sample_rate = pcm_config->sample_rate;
    }
  }

  if(audio_sample_rate == 0)
    return;

  // Group blocks by parameter_id
  std::map<uint64_t, std::vector<ParameterBlockInfo>> param_blocks;
  for(auto const &pb : state.parameterBlocks) {
    param_blocks[pb.parameter_id].push_back(pb);
  }

  // Find expected number of frames per substream.
  size_t expected_frames = 0;
  if(!state.audioFrames.empty()) {
    uint64_t ref_substream_id = state.audioFrames[0].substream_id;
    for(auto const &af : state.audioFrames) {
      if(af.substream_id == ref_substream_id) {
        expected_frames++;
      }
    }
  }

  // Maps parameter_id (key) to parameter_rate (value)
  std::map<uint64_t, uint64_t> param_rates;
  auto check_rate = [&](uint64_t parameter_id, uint64_t new_rate) {
    auto it = param_rates.find(parameter_id);
    if(it == param_rates.end()) {
      param_rates[parameter_id] = new_rate;
    } else if(it->second != new_rate) {
      out->error(
        "[Section 4] Inconsistent parameter_rate for parameter_id %lu: %lu vs %lu", parameter_id, new_rate, it->second);
    }
  };

  for(const auto &ae : state.audioElements) {
    for(const auto &param : ae.param_definitions) {
      check_rate(param.parameter_id, param.parameter_rate);
    }
  }
  for(const auto &mp : state.mixPresentations) {
    for(const auto &sm : mp.sub_mixes) {
      for(const auto &sae : sm.audio_elements) {
        check_rate(sae.element_mix_gain.parameter_id, sae.element_mix_gain.parameter_rate);
      }
      check_rate(sm.output_mix_gain.parameter_id, sm.output_mix_gain.parameter_rate);
    }
  }

  for(auto const &p : param_blocks) {
    if(p.second.size() != expected_frames) {
      out->error(
        "[Section 4] Parameter Substream %lu has %lu blocks, expected %lu (matching audio substreams)", p.first,
        p.second.size(), expected_frames);
    }

    uint64_t param_rate = 0;
    auto it = param_rates.find(p.first);
    if(it != param_rates.end()) {
      param_rate = it->second;
    }

    if(param_rate != 0) {
      // Division is guaranteed to be exact as checked in assert-parameter-definition (Section 3.6.1).
      uint64_t expected_duration = config.num_samples_per_frame * param_rate / audio_sample_rate;

      for(size_t i = 0; i < p.second.size(); ++i) {
        if(p.second[i].duration != expected_duration) {
          out->error(
            "[Section 4] Parameter Substream %lu block %zu has duration %lu, expected %lu", p.first, i,
            p.second[i].duration, expected_duration);
        }
      }
    }
  }
}

void validateDescriptorsAndDataPlacement(IReader *br, IReport *out)

{
  IamfState state;

  bool seen_data = false;
  bool seen_temporal_delimiter = false;
  size_t total_declared_audio_substreams = 0;
  std::set<uint64_t> seen_audio_frames_in_temporal_unit;
  std::set<uint64_t> seen_param_blocks_in_temporal_unit;

  auto reset_temporal_unit_state = [&]() {
    seen_audio_frames_in_temporal_unit.clear();
    seen_param_blocks_in_temporal_unit.clear();
  };

  while(!br->empty()) {
    int64_t obu_type = parseIamfObus(br, state);

    const bool is_reserved = (obu_type > OBU_IA_Audio_Frame_ID17 && obu_type < OBU_IA_Sequence_Header);
    if(is_reserved) {
      continue;
    }

    const bool started_temporal_unit =
      !seen_audio_frames_in_temporal_unit.empty() || !seen_param_blocks_in_temporal_unit.empty();
    const bool completed_temporal_unit =
      (total_declared_audio_substreams > 0 &&
       seen_audio_frames_in_temporal_unit.size() == total_declared_audio_substreams);

    const bool is_descriptor =
      (obu_type == OBU_IA_Sequence_Header || obu_type == OBU_IA_Codec_Config || obu_type == OBU_IA_Audio_Element ||
       obu_type == OBU_IA_Mix_Presentation);

    if(is_descriptor) {
      if(obu_type == OBU_IA_Audio_Element && state.obu_redundant_copy == 0) {
        if(!state.audioElements.empty()) {
          const auto &elem = state.audioElements.back();
          if(!elem.ignored) {
            total_declared_audio_substreams += elem.audio_substream_ids.size();
          }
        }
      }

      if(seen_data) {
        if(state.obu_redundant_copy != 1) {
          out->error(
            "[Section 4] Repeated Descriptors in the middle of the IA Sequence SHALL be marked as redundant "
            "(obu_redundant_copy = 1).");
        }

        if(started_temporal_unit && !completed_temporal_unit) {
          out->error(
            "[Section 4] Descriptors placed mid-sequence SHALL NOT be placed in the middle of a Temporal Unit.");
        }
      }
      continue;
    }

    // Data OBUs
    seen_data = true;

    if(obu_type == OBU_IA_Temporal_Delimiter) {
      if(!seen_temporal_delimiter) {
        if(started_temporal_unit) {
          out->error(
            "[Section 4] If Temporal Delimiter OBUs are present, the first OBU of every Temporal Unit SHALL be the "
            "Temporal Delimiter OBU.");
        }
        seen_temporal_delimiter = true;
      } else if(!completed_temporal_unit) {
        out->error("[Section 4] Incomplete set of Audio Frames in Temporal Unit.");
      }

      reset_temporal_unit_state();
      continue;
    }

    // Non-delimiter Data OBUs (Parameter Block or Audio Frame)
    if(completed_temporal_unit) {
      if(seen_temporal_delimiter) {
        out->error(
          "[Section 4] If Temporal Delimiter OBUs are present, the first OBU of every Temporal Unit SHALL be the "
          "Temporal Delimiter OBU.");
      }
      reset_temporal_unit_state();
    }

    if(obu_type == OBU_IA_Parameter_Block) {
      if(!seen_audio_frames_in_temporal_unit.empty()) {
        out->error("[Section 4] Parameter Block OBUs SHALL come first and SHALL be followed by Audio Frame OBUs.");
      }

      if(!state.parameterBlocks.empty()) {
        const uint64_t param_id = state.parameterBlocks.back().parameter_id;
        if(seen_param_blocks_in_temporal_unit.count(param_id)) {
          out->error("[Section 4] There SHALL be no redundant Parameter Block OBUs.");
        }
        seen_param_blocks_in_temporal_unit.insert(param_id);
      }
    } else if(obu_type >= OBU_IA_Audio_Frame && obu_type <= OBU_IA_Audio_Frame_ID17) {
      if(!state.audioFrames.empty()) {
        const uint64_t substream_id = state.audioFrames.back().substream_id;

        if(seen_audio_frames_in_temporal_unit.count(substream_id)) {
          if(seen_temporal_delimiter) {
            out->error(
              "[Section 4] If Temporal Delimiter OBUs are present, the first OBU of every Temporal Unit SHALL be the "
              "Temporal Delimiter OBU.");
          }
          out->error("[Section 4] Incomplete set of Audio Frames in Temporal Unit.");

          reset_temporal_unit_state();
        }
        seen_audio_frames_in_temporal_unit.insert(substream_id);
      }
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
