#include "box_reader_impl.h"
#include "av1_utils.h"
#include <cstring> // strcmp

bool checkRuleSection(const SpecDesc& spec, const char* section, Box const& root);

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
      [] (Box const& root, IReport* out)
      {
        if(isIsobmff(root))
          return;

        if(root.size < 2)
        {
          out->error("Not enough bytes(=%llu) to contain an OBU", root.size);
          return;
        }

        BoxReader br;
        br.br = BitReader { root.original, (int)root.size };

        Av1State stateUnused;
        auto obuType = parseAv1Obus(&br, stateUnused, false);

        if(!obuType)
          out->error("An AV1 stream shall contain at least one OBU but first OBU could not be parsed");
      }
    },
    {
      "Section 2.1\n"
      "Each HDR10+ OBU includes an ITU-T T.35 identifier with:\n"
      " - itu_t_t35_country_code set as 0xB5\n"
      " - itu_t_t35_terminal_provider_code set as 0x003C\n"
      " - itu_t_t35_terminal_provider_oriented_code set as 0x0001",
      [] (Box const& root, IReport* out)
      {
        if(isIsobmff(root))
          return;

        if(root.size < 2)
        {
          out->error("Not enough bytes(=%llu) to contain an OBU", root.size);
          return;
        }

        BoxReader br;
        br.br = BitReader { root.original, (int)root.size };

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
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
        }
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
      [] (Box const& root, IReport* out)
      {
        if(isIsobmff(root))
          return;

        if(root.size < 2)
        {
          out->error("Not enough bytes(=%llu) to contain an OBU", root.size);
          return;
        }

        BoxReader br;
        br.br = BitReader { root.original, (int)root.size };

        while(!br.empty())
        {
          Av1State stateUnused;
          parseAv1Obus(&br, stateUnused, false);
        }

        for(auto& sym : br.myBox.syms)
        {
          if(!strcmp(sym.name, "color_primaries"))
            if(sym.value != 9)
              out->error("color_primaries shall be set as 9 ([BT-2020]), found %d", sym.value);

          if(!strcmp(sym.name, "transfer_characteristics"))
            if(sym.value != 16)
              out->error("transfer_characteristics shall be set as 16 ([SMPTE-ST-2084] / [BT-2100]), found %d", sym.value);

          if(!strcmp(sym.name, "matrix_coefficients"))
            if(sym.value != 9)
              out->error("matrix_coefficients shall be set as 9 ([BT-2020]), found %d", sym.value);

          if(!strcmp(sym.name, "color_range"))
            if(sym.value != 1)
              out->warning("VideoFullRangeFlag shall be set as 1, found %d", sym.value);

          if(!strcmp(sym.name, "subsampling_x"))
            if(sym.value != 0)
              out->warning("subsampling_x shall be set as 0, found %d", sym.value);

          if(!strcmp(sym.name, "subsampling_y"))
            if(sym.value != 0)
              out->warning("subsampling_y shall be set as 0, found %d", sym.value);

          if(!strcmp(sym.name, "mono_chrome"))
            if(sym.value != 0)
              out->warning("mono_chrome shall be set as 0, found %d", sym.value);

          if(!strcmp(sym.name, "chroma_sample_position"))
            if(sym.value != 2)
              out->warning("chroma_sample_position shall be set as 2, found %d", sym.value);
        }
      }
    },
    {
      "Section 2.2.2\n"
      "for each frame with show_frame=1 or show_existing_frame=1, there shall be one\n"
      "and only one HDR10+ metadata OBU preceding the frame header for this frame and\n"
      "located after the last OBU of the previous frame (if any) or after the\n"
      "Sequence Header (if any) or after the start of the temporal unit (e.g. after the\n"
      "temporal delimiter, for storage formats where temporal delimiters are preserved).",
      [] (Box const& root, IReport* out)
      {
        if(isIsobmff(root))
          return;

        if(root.size < 2)
        {
          out->error("Not enough bytes(=%llu) to contain an OBU", root.size);
          return;
        }

        Av1State stateUnused;
        BoxReader br;
        br.br = BitReader { root.original, (int)root.size };

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
              out->error("The first OBU shall be a temporal unit. Aborting.");
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

#if 0 // this makes no sense

            if(av1Stream[tu][frame].size() >= 2)
              if(av1Stream[tu][frame][0].type == OBU_TEMPORAL_DELIMITER && av1Stream[tu][frame][1].isHdr10p)
                // right after the Temporal Unit Delimiter
                continue;

#endif

            bool seenFrameHeader = false, seenFrame = false, seenSeqHdr = false;

            for(auto& obu : av1Stream[tu][frame])
            {
              if(obu.type == OBU_FRAME_HEADER)
                seenFrameHeader = true;

              if(obu.isHdr10p && seenFrameHeader)
                out->error("The HR10+ metadata OBU shall precede the frame header");

              if(obu.type == OBU_FRAME)
                seenFrame = true;

              if(obu.isHdr10p && seenFrame)
                out->error("The HR10+ metadata OBU shall be located after the last OBU of the previous frame if any");

              if(obu.type == OBU_SEQUENCE_HEADER)
                seenSeqHdr = true;

              if(obu.isHdr10p && !seenSeqHdr)
                out->error("The HR10+ metadata OBU shall be located after the Sequence Header if any");
            }
          }
      }
    },
    {
      "Section 3.2\n"
      "AV1 Metadata sample group defined in [AV1-ISOBMFF] shall not be used.",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: sample groups not supported in ISOBMFF yet
      }
    },
    {
      "Section 3.2\n"
      "This specification requires that HDR10 Static Metadata [...] be unprotected",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: encryption not supported in ISOBMFF yet
        // HDR10 Static Metadata (defined as MDCV, MaxCLL and MaxFALL) may be present.
      }
    },
    {
      "Section 3.2\n"
      "This specification requires that [...] HDR10+ Metadata OBUs be unprotected ",
      [] (Box const& root, IReport* /*out*/)
      {
        if(!isIsobmff(root))
          return;

        // TODO: encryption not supported in ISOBMFF yet
      }
    },
    {
      "Section 3.3\n"
      "An ISOBMFF file or CMAF AV1 track as defined in [AV1-ISOBMFF] that also conforms\n"
      "to this specification should use the brand cdm4\n"
      "defined in [CTA-5001] in addition to the brand av01.",
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
        }
      }
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&specAv1Hdr10plus);

