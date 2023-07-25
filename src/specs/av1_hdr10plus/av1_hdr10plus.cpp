#include "box_reader_impl.h"
#include "av1_utils.h"
#include <cstring> // strcmp

bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root);
std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);

namespace
{
BitReader getData(Box const& root, IReport* out)
{
  if(!isIsobmff(root))
    return { root.original, (int)root.size }

  ;

  // FIXME: cw is not an ISOBMFF demuxer so we consider the 'mdat' box
  auto mdats = findBoxes(root, FOURCC("mdat"));

  if(mdats.size() == 1)
    return { mdats[0]->original + 8, (int)mdats[0]->size - 8 }

  ;

  out->error("%d mdat found, expected 1", mdats.size());
  return { nullptr, 0 };
}

const SpecDesc specAv1Hdr10plus =
{
  "av1hdr10plus",
  "HDR10+ AV1 Metadata Handling Specification, 7 December 2022\n"
  "https://github.com/AOMediaCodec/av1-hdr10plus/commit/63bacd21bc5f75ea6094fc11a03f0e743366fbdf\n"
  "https://aomediacodec.github.io/av1-hdr10plus/",
  { "av1isobmff" },
  {
    { // This rule does not exist in the AV1 HDR10+ spec. Should it be in some dependency?
      "Section 2.1\n"
      "An AV1 stream shall contain at least one OBU",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
        {
          out->error("Not enough bytes(=%llu) to contain an OBU", br.br.size);
          return;
        }

        out->covered();

        Av1State stateUnused;
        auto obuType = parseAv1Obus(&br, stateUnused, false);

        if(!obuType)
          out->error("An AV1 stream shall contain at least one OBU but first OBU could not be parsed");
      }
    },
    {
      "Section 2.1\n"
      "An HDR10+ Metadata OBU is defined as HDR10+ Metadata carried in a Metadata OBU.\n"
      "The metadata_type of such Metadata OBU is set to METADATA_TYPE_ITUT_T35 and the\n"
      "itu_t_t35_country_code of the corresponding Metadata ITUT T35 element is set to 0xB5.\n"
      "The remaining syntax element of Metadata ITUT T35, itu_t_t35_payload_bytes,\n"
      "is interpreted using the syntax defined in Annex S of [CTA-861], starting with\n"
      "the itu_t_t35_terminal_provider_code, and the semantics defined in [ST-2094-40].\n"
      "According to the definition of the HDR10+ Metadata, the first 6 bytes of"
      "the itu_t_t35_payload_bytes of the HDR10+ Metadata OBU are set as follows:\n"
      " - 0x003C, which corresponds to itu_t_t35_terminal_provider_code from Annex S of [CTA-861]\n"
      " - 0x0001, which corresponds to itu_t_t35_terminal_provider_oriented_code from Annex S of [CTA-861]\n"
      " - 0x4, which corresponds to application_identifier from Annex S of [CTA-861]\n"
      " - 0x1, which corresponds to application_mode from Annex S of [CTA-861]",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "itu_t_t35_country_code"))
            if(sym.value != 0xB5)
              out->error("itu_t_t35_country_code shall be set as 0xB5, found 0x%02X", sym.value);

          if(!strcmp(sym.name, "itu_t_t35_terminal_provider_code"))
            if(sym.value != 0x003C)
              out->error("itu_t_t35_terminal_provider_code shall be set as 0x003C, found 0x%04X", sym.value);

          if(!strcmp(sym.name, "itu_t_t35_terminal_provider_oriented_code"))
            if(sym.value != 0x0001)
              out->error("itu_t_t35_terminal_provider_oriented_code shall be set as 0x0001, found 0x%04X", sym.value);

          // TODO: also check application_identifier and application_mode
        }
      }
    },
    {
      "Section 2.2.1\n"
      "color_primaries = 9",
      "assert-2d0cc174",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "color_primaries"))
            if(sym.value != 9)
              out->error("color_primaries shall be set as 9 ([BT-2020]), found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.1\n"
      "transfer_characteristics = 16",
      "assert-0931ac52",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "transfer_characteristics"))
            if(sym.value != 16)
              out->error("transfer_characteristics shall be set as 16 ([SMPTE-ST-2084] / [BT-2100]), found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.1\n"
      "matrix_coefficients = 9",
      "assert-19a71368",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "matrix_coefficients"))
            if(sym.value != 9)
              out->error("matrix_coefficients shall be set as 9 ([BT-2020]), found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.1\n"
      "color_range should be set to 0",
      "assert-02249407",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "color_range"))
            if(sym.value != 0)
              out->warning("color_range should be set as 0, found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.1\n"
      "subsampling_x and subsampling_y should be set to 0",
      "assert-5230c330",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "subsampling_x"))
            if(sym.value != 0)
              out->warning("subsampling_x should be set as 0, found %d", sym.value);

          if(!strcmp(sym.name, "subsampling_y"))
            if(sym.value != 0)
              out->warning("subsampling_y should be set as 0, found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.1\n"
      "mono_chrome should be set to 0",
      "assert-4217c4a7",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "mono_chrome"))
            if(sym.value != 0)
              out->warning("mono_chrome should be set as 0, found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.1\n"
      "chroma_sample_position should be set to 2",
      "assert-5b56cde2",
      [] (Box const& root, IReport* out)
      {
        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
          out->covered();
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "chroma_sample_position"))
            if(sym.value != 2)
              out->warning("chroma_sample_position should be set as 2, found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.2\n"
      "for each frame with show_frame = 1 or show_existing_frame = 1, there shall be one\n"
      "and only one HDR10+ metadata OBU preceding the Frame Header OBU for this frame and\n"
      "located after the last OBU of the previous frame (if any) or after the\n"
      "Sequence Header OBU (if any) or after the start of the temporal unit",
      "assert-45af0987",
      [] (Box const& root, IReport* out)
      {
        if(isIsobmff(root))
          return;

        BoxReader br;
        br.br = getData(root, out);

        if(br.br.size < 2)
          return;

        struct OBU
        {
          int64_t type = 0;
          bool isHdr10p = false;
          size_t firstSymIdx = 0, lastSymIdx = 0;
        };
        struct Frame : std::vector<OBU>
        {
          bool show = false;
        };
        struct TemporalUnit : std::vector<Frame> {};
        struct AV1Stream : std::vector<TemporalUnit> {};
        AV1Stream av1Stream;
        Av1State stateUnused;

        while(!br.empty())
        {
          OBU obu;
          obu.firstSymIdx = br.myBox.syms.size();
          obu.type = parseAv1Obus(&br, stateUnused, false);
          obu.lastSymIdx = br.myBox.syms.size() - 1;

          if(obu.type == 0)
            break;

          if(obu.type == OBU_TEMPORAL_DELIMITER)
          {
            av1Stream.push_back(TemporalUnit {});
            av1Stream.back().push_back(Frame {});
          }

          if(av1Stream.empty())
          {
            if(obu.type != OBU_TEMPORAL_DELIMITER)
            {
              out->error("The first OBU shall be a temporal delimiter. Aborting.");
              break;
            }

            av1Stream.push_back(TemporalUnit {});
            av1Stream.back().push_back(Frame {});
          }

          av1Stream.back().back().push_back(obu);

          if(obu.type == OBU_FRAME)
            av1Stream.back().push_back(Frame {});
        }

        for(auto& tu : av1Stream)
          for(auto& frame : tu)
            for(auto& obu : frame)
            {
              // look for show_frame/show_existing_frame and the HDR10+ metadata OBUs
              for(size_t i = obu.firstSymIdx; i < obu.lastSymIdx; ++i)
              {
                auto& sym = br.myBox.syms[i];

                if(!strcmp(sym.name, "show_frame") || !strcmp(sym.name, "show_existing_frame"))
                  frame.show = !!sym.value;

                if(!strcmp(sym.name, "itu_t_t35_country_code"))
                  if(sym.value == 0xB5)
                    while(++i <= obu.lastSymIdx)
                    {
                      sym = br.myBox.syms[i];

                      if(!strcmp(sym.name, "itu_t_t35_terminal_provider_code"))
                        if(sym.value == 0x003C)
                          if(++i <= obu.lastSymIdx)
                          {
                            sym = br.myBox.syms[i];

                            if(!strcmp(sym.name, "itu_t_t35_terminal_provider_oriented_code"))
                              if(sym.value == 0x0001)
                                obu.isHdr10p = true;
                          }
                    }
              }
            }

        if(av1Stream.empty())
          return;

        out->covered();

        // re-aggregate the last tu&frame&obu into the last frame when there is no OBU_FRAME
        if(av1Stream.back().size() >= 2)
        {
          bool lastFrameIsNoFrame = true;

          for(auto& obu : av1Stream.back().back())
            if(obu.type == OBU_FRAME)
              lastFrameIsNoFrame = false;

          if(lastFrameIsNoFrame)
          {
            auto& prevFrame = av1Stream.back()[av1Stream.back().size() - 2];

            for(auto& obu : av1Stream.back().back())
              prevFrame.push_back(obu);

            // remove the last frame
            av1Stream.back().resize(av1Stream.back().size() - 1);
          }
        }

        for(int tu = 0; tu < (int)av1Stream.size(); ++tu)
          for(int frame = 0; frame < (int)av1Stream[tu].size(); ++frame)
          {
            if(!av1Stream[tu][frame].show)
              continue;

            int numHdr10p = 0;

            for(auto& obu : av1Stream[tu][frame])
              if(obu.isHdr10p)
                numHdr10p++;

            if(numHdr10p != 1)
            {
              out->error("There shall be one and only one HDR10+ metadata OBU. Found %d in Temporal Unit #%d (Frame #%d)", numHdr10p, tu, frame);
              continue;
            }

            bool seenFrameHeader = false, seenFrame = false, seenSeqHdr = false, hasSeqHdr = false;

            for(auto& obu : av1Stream[tu][frame])
              if(obu.type == OBU_SEQUENCE_HEADER)
                hasSeqHdr = true;

            for(auto& obu : av1Stream[tu][frame])
            {
              if(obu.type == OBU_FRAME_HEADER)
                seenFrameHeader = true;

              if(obu.isHdr10p && seenFrameHeader)
                out->error("The HR10+ metadata OBU shall precede the frame header (Temporal Unit #%d, Frame #%d)", tu, frame);

              if(obu.type == OBU_FRAME)
                seenFrame = true;

              if(obu.isHdr10p && seenFrame)
                out->error("The HR10+ metadata OBU shall be located after the last OBU of the previous frame if any (Temporal Unit #%d, Frame #%d)", tu, frame);

              if(obu.type == OBU_SEQUENCE_HEADER)
                seenSeqHdr = true;

              if(obu.isHdr10p && hasSeqHdr && !seenSeqHdr)
                out->error("The HR10+ metadata OBU shall be located after the Sequence Header if any (Temporal Unit #%d, Frame #%d)", tu, frame);
            }
          }
      }
    },
    {
      "Section 2.2.2\n"
      "HDR10+ Metadata OBUs are not provided when show_frame = 0",
      "assert-a575dc54",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: implement me
      }
    },
    {
      "Section 2.2.2\n"
      "For non-layered streams, there is only one HDR10+ Metadata OBU per temporal unit",
      "assert-797eb19e",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: implement me
      }
    },
    {
      "Section 3.1\n"
      "For formats that use the AV1CodecConfigurationRecord when storing\n"
      "[AV1] bitstreams (e.g. ISOBMFF and MPEG-2 TS), HDR10+ Metadata OBUs\n"
      "shall not be present in the configOBUs field of\n"
      "the AV1CodecConfigurationRecord",
      "assert-aa071f33",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: implement me
      }
    },
    {
      "Section 3.2\n"
      "AV1 Metadata sample group defined in [AV1-ISOBMFF] shall not be used.",
      "assert-398f68cd",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: sample groups not supported in ISOBMFF yet
        // out->covered();
      }
    },
    {
      "Section 3.2\n"
      "HDR10 Static Metadata and HDR10+ Metadata OBUs are unprotected",
      "assert-d451561e",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: encryption not supported in ISOBMFF yet
        // HDR10 Static Metadata (defined as MDCV, MaxCLL and MaxFALL) may be present.
        // out->covered();
      }
    },
    {
      "Section meta 3.2\n" // add 'meta' to avoid an infinite recursion
      "An ISOBMFF file or CMAF AV1 track as defined in [AV1-ISOBMFF] that also\n"
      "conforms to this specification (i.e. that contains HDR10+ metadata OBUs and\n"
      "complies to the constraints from this specification) should use the brand cdm4\n"
      "defined in [CTA-5001] in addition to the brand av01",
      "assert-c56194aa",
      [] (Box const& root, IReport* out)
      {
        if(!isIsobmff(root))
          return;

        if(checkRuleSection(specAv1Hdr10plus, "2.", root) && checkRuleSection(specAv1Hdr10plus, "3.2", root))
        {
          bool cdm4Found = false;

          for(auto& box : root.children)
            if(box.fourcc == FOURCC("ftyp"))
              for(auto& sym : box.syms)
                if(!strcmp(sym.name, "compatible_brand"))
                  if(toString((uint32_t)sym.value) == "cdm4")
                    if(!cdm4Found)
                      out->warning("'cdm4' brand should be present but is not in the 'ftyp' compatible_brand list");

          out->covered();
        }
      }
    },
    {
      "Section 3.2\n"
      "If the brand cdm4 is used in conjunction with [AV1] bitstreams, the constraints\n"
      "defined in this specification shall be respected",
      "assert-3a8897d6",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // This should already be implemented in here by evaluating all other rules. We keep it in here to satisfy the python script.
      }
    },
    {
      "Section 3.3\n"
      "[DASH] content following [DASH-IOP] should include a Supplemental Descriptor\n"
      "with @schemeIdUri set to http://dashif.org/metadata/hdr and @value set to\n"
      "SMPTE2094-40 in manifest files",
      "assert-622a560f",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // This is out of scope for ComplianceWarden. We keep it in here to satisfy the python script.
      }
    },
    {
      "Section 3.3\n"
      "The value of the Codecs Parameter String for [AV1] bitstreams that is used when\n"
      "using HTTP streaming technologies shall remain unchanged\n"
      "when HDR10+ Metadata OBUs are included in the [AV1] stream",
      "assert-91363c5f",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // This is out of scope for ComplianceWarden. We keep it in here to satisfy the python script.
      }
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&specAv1Hdr10plus);
}

