#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"
#include "utils/iamf_utils.h"

#include <cstring>
#include <set>

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
