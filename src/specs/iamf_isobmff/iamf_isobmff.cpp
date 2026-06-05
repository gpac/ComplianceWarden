#include "core/box_reader_impl.h"
#include "core/fourcc.h"
#include "core/spec.h"
#include "utils/iamf_utils.h"
#include "utils/isobmff_get_data.h"

#include <cstring>

std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc);

namespace
{

std::initializer_list<RuleDesc> rulesIamfIsobmff = {
  { "Section 6.2.3\n"
    "An 'iamf' sample entry SHALL contain an 'iacb' box.",
    "assert-iamf-sample-entry-has-iacb",
    [](Box const &root, IReport *out) {
      auto iamfBoxes = findBoxes(root, FOURCC("iamf"));
      for(auto const &iamfBox : iamfBoxes) {
        out->covered();
        auto iacbBoxes = findBoxes(*iamfBox, FOURCC("iacb"));
        if(iacbBoxes.empty()) {
          out->error("Sample entry 'iamf' does not contain an 'iacb' box");
        }
      }
    } },
  { "Section 6.2.3\n"
    "The channelcount field of AudioSampleEntry SHALL be set to 0. "
    "The samplerate field of AudioSampleEntry SHALL be set to 0. "
    "There SHALL be no SamplingRateBox.",
    "assert-iamf-audio-sample-entry-fields-zero",
    [](Box const &root, IReport *out) {
      auto iamfBoxes = findBoxes(root, FOURCC("iamf"));
      for(auto const &iamfBox : iamfBoxes) {
        out->covered();
        uint16_t channelcount = 0;
        uint32_t samplerate = 0;
        for(auto &sym : iamfBox->syms) {
          if(!strcmp(sym.name, "channelcount")) {
            channelcount = sym.value;
          }
          if(!strcmp(sym.name, "samplerate")) {
            samplerate = sym.value;
          }
        }
        if(channelcount != 0) {
          out->error("Sample entry 'iamf' channelcount is %u, SHALL be 0", channelcount);
        }
        if(samplerate != 0) {
          out->error("Sample entry 'iamf' samplerate is %u, SHALL be 0", samplerate);
        }

        auto sratBoxes = findBoxes(*iamfBox, FOURCC("srat"));
        if(!sratBoxes.empty()) {
          out->error("Sample entry 'iamf' SHALL NOT contain a SamplingRateBox ('srat')");
        }
      }
    } },
  { "Section 6.2.4\n"
    "The Configuration OBUs contained in the 'iacb' box SHALL comply with the IAMF specification.",
    "assert-iacb-config-obus",
    [](Box const &root, IReport *out) {
      auto iacbBoxes = findBoxes(root, FOURCC("iacb"));
      for(auto const &iacbBox : iacbBoxes) {
        out->covered();

        BoxReader br;
        br.br = BitReader{ iacbBox->original, (int)iacbBox->size };
        uint64_t size = br.br.u(32);
        br.br.u(32); // fourcc
        if(size == 1) {
          br.br.u(64);
        }

        IamfState state;
        parseIacb(&br, state);

        if(!validateProfiles(state, out)) {
          continue;
        }

        validateFirstObuIsSeqHdr(state, out);
        validateSequenceHeaderIaCode(state, out);
        validateSequenceHeaderTrimming(state, out);
        validateCodecConfig(state, out);
        validateAudioElement(state, out);
        validateParameterDefinitions(state, out);
        validateScalableChannelLayoutConfig(state, out);
        validateScalableChannelLayoutGeneration(state, out);
        validateScalableChannelGroupFormat(state, out);
        validateAmbisonicsConfig(state, out);
        validateMixPresentation(state, out);
        validateMixPresentationLoudness(state, out);
        validateMixPresentationTags(state, out);
        validateLpcmSpecific(state, out);
        validateCommonProfileRestrictions(state, out);
        validateProfileRestrictions(state, out, 0);
        validateProfileRestrictions(state, out, 1);
        validateProfileRestrictions(state, out, 2);
        validateDescriptorObusOrder(state, out);
      }
    } },
};

static const SpecDesc specIamfIsobmff = {
  "iamf_isobmff", "IAMF in ISOBMFF", { "isobmff" }, rulesIamfIsobmff, nullptr,
};

static auto const registered = registerSpec(&specIamfIsobmff);

} // namespace
