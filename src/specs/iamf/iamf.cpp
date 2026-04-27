#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"
#include "utils/iamf_utils.h"

#include <cstring>

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
