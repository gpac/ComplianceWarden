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

    state.audioFrames.push_back({ static_cast<uint64_t>(obu_type), substream_id });

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
