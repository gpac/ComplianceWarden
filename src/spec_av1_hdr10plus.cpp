#include "spec.h"

static const SpecDesc specAv1Hdr10plus =
{
  "av1hdr10plus",
  "HDR10+ AV1 Metadata Handling Specification, 8 December 2021\n"
  "https://aomediacodec.github.io/av1-hdr10plus/",
  { "isobmff" },
  {
    {
      "Section 2.1\n"
      "An AV1 stream shall contain at least one OBU",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
      }
    },
    {
      "Section 2.1\n"
      "Each HDR10+ OBU includes an ITU-T T.35 identifier with:\n"
      " - itu_t_t35_country_code set as 0xB5\n"
      " - itu_t_t35_terminal_provider_code set as 0x003C\n"
      " - itu_t_t35_terminal_provider_oriented_code set as 0x0001",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "Streams shall use the following values for the AV1 color_config:\n"
      " - color_primaries = 9 ([BT-2020])\n"
      " - transfer_characteristics = 16 ([SMPTE-ST-2084] / [BT-2100])\n"
      " - matrix_coefficients = 9 ([BT-2020])\n"
      "Additionally, the following recommendations apply:\n"
      " - VideoFullRangeFlag should be set to 0\n"
      " - subsampling_x and subsampling_y should be set to 0\n"
      " - mono_chrome should be 0\n"
      " - chroma_sample_position should be set to 2",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
      }
    },
    {
      "Section 2.2.2\n"
      "for each frame with show_frame=1 or show_existing_frame=1, there shall be one\n"
      "and only one HDR10+ metadata OBU preceding the frame header for this frame and\n"
      "located after the last OBU of the previous frame (if any) or after the\n"
      "Sequence Header (if any) or after the start of the temporal unit (e.g. after the\n"
      "temporal delimiter, for storage formats where temporal delimiters are preserved).",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
        // A TU contains a series of OBUs starting from a Temporal Delimiter, optional sequence headers, optional metadata OBUs,
        // a sequence of one or more frame headers, each followed by zero or more tile group OBUs as well as optional padding OBUs.
        /*
           ASSERTION 2.2.2a
           GIVEN: an AV1 stream with HDR10+
           WHEN: each frame in the stream is observed
           AND: the frame has show_frame set to 1
           OR: the frame has show_existing_frame set to 1
           THEN: exactly one HDR10+ metadata OBU exists
           AND: it is placed within the frame as defined in Section 2.2.2.

           ASSERTION 2.2.2b
           GIVEN: an AV1 stream with HDR10+
           WHEN: each frame in the stream is observed
           AND: the frame has show_frame set to 0
           THEN: no HDR10+ metadata OBU exists within that frame.

           ASSERTION 2.2.2c
           GIVEN: a non-layered AV1 stream with HDR10+
           WHEN: the stream is observed
           THEN: exactly one HDR10+ metadata OBU exists per TU.
         */
      }
    },
    {
      "Section 3.2\n"
      "AV1 Metadata sample group defined in [AV1-ISOBMFF] shall not be used.",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
      }
    },
    {
      "Section 3.2\n"
      "An ISOBMFF file or CMAF AV1 track as defined in [AV1-ISOBMFF] that also conforms\n"
      "to this specification should use the brand cdm4\n"
      "defined in [CTA-5001] in addition to the brand av01.",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
        // Comment: these rules are triggered on automatic detection... should we add something based on rules in cw?
      }
    },
    {
      "Section 3.2\n"
      "If the brand cdm4 is used in conjunction with AV1 streams, the constraints\n"
      "defined in this specification shall be respected.",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
      }
    },
    {
      "Section 3.2\n"
      "This specification requires that HDR10 Static Metadata [...] be unprotected",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
        // HDR10 Static Metadata (defined as MDCV, MaxCLL and MaxFALL) may be present.
      }
    },
    {
      "Section 3.2\n"
      "This specification requires that [...] HDR10+ Metadata OBUs be unprotected",
      [] (Box const & /*root*/, IReport* /*out*/)
      {
        // TODO
      }
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&specAv1Hdr10plus);

