#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"
#include "utils/iamf_utils.h"

#include <cstring>
#include <set>
#include <string>

namespace
{

bool probeIamf(Box const &root, BoxReader &br, IamfState &state, IReport *out)
{
  br.br = { root.original, (int)root.size };
  if(br.empty()) {
    if(out)
      out->error("File is empty");
    return false;
  }
  parseIamfObus(&br, state);
  return true;
}

std::initializer_list<RuleDesc> rulesIamf = {
  { "Section 3.2\n"
    "obu_trimming_status_flag is set to 0 for an IA Sequence Header OBU.",
    "assert-seq-hdr-trimming-flag-zero",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;
      out->covered();
      validateSequenceHeaderTrimming(state, out);
    } },
  { "Section 3.4\n"
    "The first OBU in an IA Sequence SHALL have obu_type = OBU_IA_Sequence_Header.",
    "assert-first-obu-is-seq-hdr",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, out))
        return;
      out->covered();
      validateFirstObuIsSeqHdr(state, out);
    } },
  { "Section 3.4\n"
    "ia_code is a 'four-character code' (4CC), iamf.",
    "assert-ia-code-is-iamf",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;
      out->covered();
      validateSequenceHeaderIaCode(state, out);
    } },
  { "Section 3.5\n"
    "Codec Config OBU checks:\n"
    "- There SHALL be exactly one Codec Config OBU with a given identifier in a set of Descriptors.\n"
    "- codec_id indicates a ‘four-character code’ (4CC) to identify the codec. Supported values: Opus, mp4a, fLaC, "
    "ipcm.\n"
    "- num_samples_per_frame SHALL NOT be set to zero.\n"
    "- audio_roll_distance SHALL always be a negative value or zero.\n"
    "- audio_roll_distance SHALL be set to -R when codec_id is set to Opus, where R = ceil(3840 / "
    "num_samples_per_frame).\n"
    "- audio_roll_distance SHALL be set to -1 when codec_id is set to mp4a.\n"
    "- audio_roll_distance SHALL be set to 0 when codec_id is set to fLaC or ipcm.",
    "assert-codec-config-obu",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateCodecConfig(state, out);
    } },
  { "Section 3.6\n"
    "Audio Element OBU checks:\n"
    "- audio_element_id SHALL be unique within an IA Sequence.\n"
    "- num_substreams SHALL NOT be set to 0.\n"
    "- When audio_element_type = 0 (CHANNEL_BASED), num_parameters SHALL be set to 0, 1, or 2.\n"
    "- When audio_element_type = 1 (SCENE_BASED), num_parameters SHALL be set to 0.\n"
    "- The type PARAMETER_DEFINITION_MIX_GAIN SHALL NOT be present in Audio Element OBU.\n"
    "- The parameter type SHALL NOT be duplicated in one Audio Element OBU.\n"
    "- When codec_id = fLaC or ipcm, the type PARAMETER_DEFINITION_RECON_GAIN SHALL NOT be present.\n"
    "- When num_layers > 1, the type PARAMETER_DEFINITION_RECON_GAIN SHALL be present.\n"
    "- When highest loudspeaker_layout does not correspond to Mono, Stereo, or 3.1.2 and num_layers > 1, both "
    "PARAMETER_DEFINITION_DEMIXING and PARAMETER_DEFINITION_RECON_GAIN SHALL be present.",
    "assert-audio-element-obu",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();
      validateAudioElement(state, out);
    } },
  { "Section 3.6.1\n"
    "Parameter Definition Syntax and Semantics checks:\n"
    "- There SHALL be one unique parameter_id per Parameter Substream.\n"
    "- duration SHALL NOT be set to 0.\n"
    "- When constant_subblock_duration is equal to 0, the summation of all subblock_duration in this parameter block "
    "SHALL be equal to duration.\n"
    "- subblock_duration SHALL NOT be set to 0.",
    "assert-parameter-definition",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateParameterDefinitions(state, out);
    } },
  { "Section 3.6.2\n"
    "Scalable Channel Layout Config checks:\n"
    "- num_layers SHALL NOT be set to 0.\n"
    "- num_layers maximum value SHALL be 6.\n"
    "- For an expanded channel layout defined in expanded_loudspeaker_layout, num_layers SHALL be set to 1.\n"
    "- If loudspeaker_layout is set to Binaural (9), num_layers SHALL be set to 1.\n"
    "- substream_count SHALL be greater than or equal to coupled_substream_count for all layers.\n"
    "- substream_count SHALL NOT be set to 0 for any layer.\n"
    "- The sum of substream_count across all layers SHALL be equal to num_substreams of the Audio Element.",
    "assert-scalable-channel-layout-config",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateScalableChannelLayoutConfig(state, out);
    } },
  { "Section 3.6.3\n"
    "Channel Layout Generation Rule checks:\n"
    "- For scalable channel audio, channel layouts SHALL follow non-decreasing order of surround, LFE and height "
    "channels.\n"
    "- Duplicate layers are NOT allowed.",
    "assert-scalable-channel-layout-generation",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateScalableChannelLayoutGeneration(state, out);
    } },
  { "Section 3.6.3\n"
    "Channel Group Format checks:\n"
    "- For scalable channel audio, substream_count and coupled_substream_count SHALL match expected values derived "
    "from layout transitions.",
    "assert-scalable-channel-group-format",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateScalableChannelGroupFormat(state, out);
    } },
  { "Section 3.6.4\n"
    "Ambisonics Config checks:\n"
    "- ambisonics_mode SHALL be 0 (MONO) or 1 (PROJECTION).\n"
    "- output_channel_count SHALL be (1 + n)^2 for n = 0, 1, 2, ..., 14.\n"
    "- substream_count SHALL be the same as num_substreams in this OBU.\n"
    "- For PROJECTION mode, coupled_substream_count SHALL be less than or equal to substream_count.",
    "assert-ambisonics-config",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateAmbisonicsConfig(state, out);
    } },
  { "Section 3.7\n"
    "Mix Presentation OBU checks:\n"
    "- mix_presentation_id SHALL be unique within an IA Sequence.\n"
    "- num_sub_mixes SHALL NOT be set to 0.\n"
    "- num_audio_elements SHALL NOT be set to 0.\n"
    "- There SHALL be no duplicate values of audio_element_id within one Mix Presentation.\n"
    "- Each sub-mix SHALL include loudness for Stereo.",
    "assert-mix-presentation-obu",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateMixPresentation(state, out);
    } },
  { "Section 3.7.4\n"
    "Loudness Info checks:\n"
    "- There SHALL be no duplicate values of anchor_element within one LoudnessInfo.",
    "assert-mix-presentation-loudness",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateMixPresentationLoudness(state, out);
    } },
  { "Section 3.7.5\n"
    "Mix Presentation Tags checks:\n"
    "- There SHALL be at most one instance of content_language tag.\n"
    "- content_language tag value SHALL conform to ISO-639-2-Codes (3-letter code).",
    "assert-mix-presentation-tags",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateMixPresentationTags(state, out);
    } },
  { "Section 3.8\n"
    "Parameter Block OBU checks:\n"
    "- duration SHALL NOT be set to 0.\n"
    "- subblock_duration SHALL NOT be set to 0.\n"
    "- The summation of all subblock_duration SHALL be equal to duration.",
    "assert-parameter-block-obu",
    [](Box const &root, IReport *out) {
      IamfState state;
      BoxReader br;
      if(!probeIamf(root, br, state, nullptr))
        return;

      while(!br.empty()) {
        parseIamfObus(&br, state);
      }

      out->covered();

      validateParameterBlocks(state, out);
    } }
};

static const SpecDesc specIamf = {
  "iamf",
  "IAMF v1.1.0\n"
  "https://aomediacodec.github.io/iamf/v1.1.0.html",
  {}, // no dependencies
  rulesIamf,
  nullptr, // valid
};

static auto const registered = registerSpec(&specIamf);

} // namespace
