#include "av1_utils.h"
#include "box_reader_impl.h" // BoxReader
#include <memory> // make_unique
#include <stdexcept>

namespace
{
#define READ_UNTIL_NEXT_BYTE(readBits) \
  if(readBits % 8){ \
    auto remainderBits = 8 - (readBits % 8); \
    br->sym("bits", remainderBits); \
    readBits += remainderBits; \
  }

struct ReaderBits : IReader
{
  ReaderBits(IReader* delegate) : delegate(delegate) {}

  virtual ~ReaderBits() {}

  bool empty()
  {
    return delegate->empty();
  }

  int64_t sym(const char* name, int bits)
  {
    count += bits;
    return delegate->sym(name, bits);
  }

  void box()
  {
    delegate->box();
  }

  IReader* delegate = nullptr;
  int64_t count = 0;
};

void parseAv1ColorConfig(ReaderBits* br, int64_t seq_profile, AV1CodecConfigurationRecord& av1c, int64_t& color_range)
{
  av1c.high_bitdepth = br->sym("high_bitdepth", 1);
  int BitDepth = 8;

  if(seq_profile == 2 && av1c.high_bitdepth)
  {
    av1c.twelve_bit = br->sym("twelve_bit", 1);
    BitDepth = av1c.twelve_bit ? 12 : 10;
  }
  else if(seq_profile <= 2)
  {
    BitDepth = av1c.high_bitdepth ? 10 : 8;
  }

  if(seq_profile == 1)
  {
    // av1c.mono_chrome = br->sym("mono_chrome", 0); // = 0;
  }
  else
  {
    av1c.mono_chrome = br->sym("mono_chrome", 1);
  }

  // NumPlanes = mono_chrome ? 1 : 3;
  auto color_description_present_flag = br->sym("color_description_present_flag", 1);
  uint8_t color_primaries, transfer_characteristics, matrix_coefficients;

  if(color_description_present_flag)
  {
    color_primaries = br->sym("color_primaries", 8);
    transfer_characteristics = br->sym("transfer_characteristics", 8);
    matrix_coefficients = br->sym("matrix_coefficients", 8);
  }
  else
  {
    color_primaries = 2;// CP_UNSPECIFIED;
    transfer_characteristics = 2;// TC_UNSPECIFIED;
    matrix_coefficients = 2;// MC_UNSPECIFIED;
  }

  if(av1c.mono_chrome)
  {
    color_range = br->sym("color_range", 1);
    av1c.chroma_subsampling_x = 1;
    av1c.chroma_subsampling_y = 1;
    av1c.chroma_sample_position = 0;// CSP_UNKNOWN;
    br->sym("separate_uv_delta_q", 0); // = 0;
    return;
  }
  else if(color_primaries == 0 /*CP_BT_709*/ &&
          transfer_characteristics == 13 /*TC_SRGB*/ &&
          matrix_coefficients == 0 /*MC_IDENTITY*/)
  {
    color_range = 1;
    av1c.chroma_subsampling_x = 0;
    av1c.chroma_subsampling_y = 0;
  }
  else
  {
    color_range = br->sym("color_range", 1);

    if(seq_profile == 0)
    {
      av1c.chroma_subsampling_x = 1;
      av1c.chroma_subsampling_y = 1;
    }
    else if(seq_profile == 1)
    {
      av1c.chroma_subsampling_x = 0;
      av1c.chroma_subsampling_y = 0;
    }
    else
    {
      if(BitDepth == 12)
      {
        av1c.chroma_subsampling_x = br->sym("subsampling_x", 1);

        if(av1c.chroma_subsampling_x)
          av1c.chroma_subsampling_y = br->sym("subsampling_y", 1);
        else
          av1c.chroma_subsampling_y = 0;
      }
      else
      {
        av1c.chroma_subsampling_x = 1;
        av1c.chroma_subsampling_y = 0;
      }
    }

    if(av1c.chroma_subsampling_x && av1c.chroma_subsampling_y)
    {
      br->sym("chroma_sample_position", 2);
    }
  }

  br->sym("separate_uv_delta_q", 1);
}

int parseAv1SeqHdr(IReader* reader, Av1State& state)
{
  auto br = std::make_unique<ReaderBits>(reader);

  state.av1c.seq_profile = br->sym("seq_profile", 3);
  br->sym("still_picture", 1);
  state.reduced_still_picture_header = br->sym("reduced_still_picture_header", 1);

  if(state.reduced_still_picture_header)
  {
    br->sym("timing_info_present_flag", 0); // =0
    auto decoder_model_info_present_flag = br->sym("decoder_model_info_present_flag", 0); // =0

    if(decoder_model_info_present_flag != 0)
      throw std::runtime_error("Unimplemented decoder_model_info_present_flag != 0. Aborting.");

    br->sym("initial_display_delay_present_flag", 0); // =0
    br->sym("operating_points_cnt_minus_1", 0); // =0
    br->sym("operating_point_idc_0", 0); // =0
    state.av1c.seq_level_idx_0 = br->sym("seq_level_idx_0", 5);
    state.av1c.seq_tier_0 = br->sym("seq_tier_0", 0); // =0
    br->sym("decoder_model_present_for_this_op_0", 0); // =0
    br->sym("initial_display_delay_present_for_this_op_0", 0); // =0
  }
  else
  {
    auto timing_info_present_flag = br->sym("timing_info_present_flag", 1);

    if(timing_info_present_flag != 0) // timing info and consequence in uncompressed header not implemented
      throw std::runtime_error("Unimplemented timing_info_present_flag != 0. Aborting.");

    auto initial_display_delay_present_flag = br->sym("initial_display_delay_present_flag", 1);

    if(initial_display_delay_present_flag != 0)
      throw std::runtime_error("Unimplemented initial_display_delay_present_flag != 0. Aborting.");

    auto operating_points_cnt_minus_1 = br->sym("operating_points_cnt_minus_1", 5);

    for(int i = 0; i <= operating_points_cnt_minus_1; i++)
    {
      br->sym("operating_point_idc[i])", 12);
      auto seq_level_idx = br->sym("seq_level_idx[i]", 5);
      int64_t seq_tier = 0;

      if(seq_level_idx > 7)
      {
        seq_tier = br->sym("seq_tier[i]", 1);
      }
      else
      {
        seq_tier = br->sym("seq_tier[i]", 0); // =0
      }

      if(i == 0)
      {
        state.av1c.seq_level_idx_0 = seq_level_idx;
        state.av1c.seq_tier_0 = seq_tier;
      }

      // Not covered: there is an assert ensuring decoder_model_info_present_flag == false.
      /*
         if ( decoder_model_info_present_flag ) {
         decoder_model_present_for_this_op[ i ] f(1)
         if ( decoder_model_present_for_this_op[ i ] ) {
          operating_parameters_info( i )
         }
         } else {
         decoder_model_present_for_this_op[ i ] = 0
         }
       */
      // Not covered: there is an assert ensuring initial_display_delay_present_flag == false.
      /*
         if ( initial_display_delay_present_flag ) {
         initial_display_delay_present_for_this_op[ i ] f(1)
         if ( initial_display_delay_present_for_this_op[ i ] ) {
          initial_display_delay_minus_1[ i ] f(4)
         }
         }
       */
    }
  }

  auto frame_width_bits_minus_1 = br->sym("frame_width_bits_minus_1", 4);
  auto frame_height_bits_minus_1 = br->sym("frame_height_bits_minus_1", 4);
  br->sym("max_frame_width_minus_1", frame_width_bits_minus_1 + 1);
  br->sym("max_frame_height_minus_1", frame_height_bits_minus_1 + 1);

  if(state.reduced_still_picture_header)
  {
    state.frame_id_numbers_present_flag = 0;
  }
  else
  {
    state.frame_id_numbers_present_flag = br->sym("frame_id_numbers_present_flag", 1);
  }

  if(state.frame_id_numbers_present_flag)
  {
    state.delta_frame_id_length_minus_2 = br->sym("delta_frame_id_length_minus_2", 4);
    state.additional_frame_id_length_minus_1 = br->sym("additional_frame_id_length_minus_1", 3);
  }

  br->sym("use_128x128_superblock", 1);
  br->sym("enable_filter_intra", 1);
  br->sym("enable_intra_edge_filter", 1);

  if(state.reduced_still_picture_header)
  {
    br->sym("enable_interintra_compound", 0); // =0
    br->sym("enable_masked_compound", 0); // =0
    br->sym("enable_warped_motion", 0); // =0
    br->sym("enable_dual_filter", 0); // =0
    br->sym("enable_order_hint", 0); // =0
    br->sym("enable_jnt_comp", 0); // =0
    br->sym("enable_ref_frame_mvs", 0); // =0
    br->sym("seq_force_screen_content_tools", 0); // 0, should be SELECT_SCREEN_CONTENT_TOOLS(2)
    br->sym("seq_force_integer_mv", 0); // 0, should be SELECT_INTEGER_MV(2)
    br->sym("OrderHintBits", 0); // =0
  }
  else
  {
    br->sym("enable_interintra_compound", 1);
    br->sym("enable_masked_compound", 1);
    br->sym("enable_warped_motion", 1);
    br->sym("enable_dual_filter", 1);
    auto enable_order_hint = br->sym("enable_order_hint", 1);

    if(enable_order_hint)
    {
      br->sym("enable_jnt_comp", 1);
      br->sym("enable_ref_frame_mvs", 1);
    }
    else
    {
      // enable_jnt_comp = 0
      // enable_ref_frame_mvs = 0
    }

    auto seq_choose_screen_content_tools = br->sym("seq_choose_screen_content_tools", 1);

    int64_t seq_force_screen_content_tools = 0;

    if(seq_choose_screen_content_tools)
    {
      seq_force_screen_content_tools = 2; // SELECT_SCREEN_CONTENT_TOOLS
    }
    else
    {
      seq_force_screen_content_tools = br->sym("seq_force_screen_content_tools", 1);
    }

    if(seq_force_screen_content_tools > 0)
    {
      auto seq_choose_integer_mv = br->sym("seq_choose_integer_mv", 1);

      if(seq_choose_integer_mv)
      {
        // seq_force_integer_mv = SELECT_INTEGER_MV
      }
      else
      {
        br->sym("seq_force_integer_mv", 1);
      }
    }
    else
    {
      // seq_force_integer_mv = SELECT_INTEGER_MV
    }

    if(enable_order_hint)
    {
      br->sym("order_hint_bits_minus_1", 3);
      // OrderHintBits = order_hint_bits_minus_1 + 1
    }
    else
    {
      // OrderHintBits = 0
    }
  }

  br->sym("enable_superres", 1);
  br->sym("enable_cdef", 1);
  br->sym("enable_restoration", 1);
  parseAv1ColorConfig(br.get(), state.av1c.seq_profile, state.av1c, state.color_range);
  br->sym("film_grain_params_present", 1);

  auto readBits = br->count;
  READ_UNTIL_NEXT_BYTE(readBits);
  return readBits / 8;
}

int parseAv1UncompressedHeader(IReader* reader, Av1State const& state)
{
  auto br = std::make_unique<ReaderBits>(reader);

  int idLen = 0;

  if(state.frame_id_numbers_present_flag)
  {
    idLen = state.additional_frame_id_length_minus_1 + state.delta_frame_id_length_minus_2 + 3;
  }

  if(state.reduced_still_picture_header)
  {
    br->sym("key_frame", 0);
    br->sym("show_frame", 0);
    return 0;
  }
  else
  {
    auto show_existing_frame = br->sym("show_existing_frame", 1);

    if(show_existing_frame)
    {
      br->sym("frame_to_show_map_idx", 3);

      // Not covered: there is an assert in the sequence header.
      // if ( decoder_model_info_present_flag && !equal_picture_interval ) {
      // temporal_point_info( )
      // }
      // refresh_frame_flags = 0

      if(state.frame_id_numbers_present_flag)
      {
        br->sym("display_frame_id", idLen);
      }

      // we don't refresh RefFrameType
      // frame_type = RefFrameType[frame_to_show_map_idx];

      auto readBits = br->count;
      READ_UNTIL_NEXT_BYTE(readBits);
      return readBits / 8;
    }

    br->sym("frame_type", 2);
    br->sym("show_frame", 1);

    auto readBits = br->count;
    READ_UNTIL_NEXT_BYTE(readBits);
    return readBits / 8;
  }
}

uint64_t leb128_read(IReader* br)
{
  uint64_t value = 0;

  for(int i = 0; i < 8; i++)
  {
    uint8_t leb128_byte = br->sym("leb128_byte", 8);
    value |= (((uint64_t)(leb128_byte & 0x7f)) << (i * 7));

    if(!(leb128_byte & 0x80))
      break;
  }

  return value;
}

enum
{
  METADATA_TYPE_HDR_CLL = 1,
  METADATA_TYPE_HDR_MDCV = 2,
  METADATA_TYPE_ITUT_T35 = 4
};

void parseMetadataItutT35(ReaderBits* br, Av1State & /*state*/)
{
  auto const itu_t_t35_country_code = br->sym("itu_t_t35_country_code", 8);

  if(itu_t_t35_country_code == 0xFF)
    br->sym("itu_t_t35_country_code_extension_byte", 8);

  // itu_t_t35_payload_bytes
  br->sym("itu_t_t35_terminal_provider_code", 16);
  br->sym("itu_t_t35_terminal_provider_oriented_code", 16);
}

void parseMetadataHdrCll(ReaderBits* br, Av1State & /*state*/)
{
  br->sym("max_cll", 16);
  br->sym("max_fallmax_fall", 16);
}

void parseMetadataHdrMdcv(ReaderBits* br, Av1State & /*state*/)
{
  for(auto i = 0; i < 3; ++i)
  {
    br->sym("primary_chromaticity_x", 16);
    br->sym("primary_chromaticity_y", 16);
  }

  br->sym("white_point_chromaticity_x", 16);
  br->sym("white_point_chromaticity_y", 16);
  br->sym("luminance_max", 32);
  br->sym("luminance_min", 32);
}

int parseAv1MetadataObu(IReader* reader, Av1State& state)
{
  auto br = std::make_unique<ReaderBits>(reader);

  auto const metadata_type = leb128_read(br.get());

  if(metadata_type == METADATA_TYPE_ITUT_T35)
    parseMetadataItutT35(br.get(), state);
  else if(metadata_type == METADATA_TYPE_HDR_CLL)
    parseMetadataHdrCll(br.get(), state);
  else if(metadata_type == METADATA_TYPE_HDR_MDCV)
    parseMetadataHdrMdcv(br.get(), state);

  auto readBits = br->count;
  READ_UNTIL_NEXT_BYTE(readBits);
  return readBits / 8;
}
} // anonymous namespace

int64_t parseAv1Obus(IReader* br, Av1State& state, bool storeUnparsed)
{
  br->sym("obu", 0); // virtual OBU separator
  br->sym("forbidden", 1);
  auto obu_type = br->sym("obu_type", 4);
  auto obu_extension_flag = br->sym("obu_extension_flag", 1);
  auto obu_has_size_field = br->sym("obu_has_size_field", 1);

  br->sym("obu_reserved_1bit", 1);

  if(obu_extension_flag)
  {
    br->sym("temporal_id", 3);
    br->sym("spatial_id", 2);
    br->sym("extension_header_reserved_3bits", 3);
  }

  long long unsigned obuSize = obu_has_size_field ? leb128_read(br) : INT64_MAX;
  switch(obu_type)
  {
  case OBU_SEQUENCE_HEADER:
    br->sym("seqhdr", 0);
    obuSize -= parseAv1SeqHdr(br, state);
    br->sym("/seqhdr", 0);
    break;
  case OBU_FRAME_HEADER: case OBU_REDUNDANT_FRAME_HEADER: case OBU_FRAME:
    obuSize -= parseAv1UncompressedHeader(br, state);
    break;
  case OBU_METADATA:
    obuSize -= parseAv1MetadataObu(br, state);
    break;
  default: break;
  }

  while(obuSize-- > 0)
  {
    if(br->empty())
    {
      if(obu_has_size_field)
        fprintf(stderr, "Incomplete OBU (remaining to read=%llu)\n", obuSize + 1);

      return obu_type;
    }

    if(storeUnparsed)
    {
      br->sym("byte", 8);
    }
    else
    {
      auto boxReader = dynamic_cast<BoxReader*>(br);
      boxReader->br.m_pos += 8; // don't store for performance reasons - data is still accessible from the original parsing (e.g Box)
    }
  }

  return obu_type;
}

void parseAv1C(IReader* br)
{
  br->sym("marker", 1);
  br->sym("version", 7);
  br->sym("seq_profile", 3);
  br->sym("seq_level_idx_0", 5);
  br->sym("seq_tier_0", 1);
  br->sym("high_bitdepth", 1);
  br->sym("twelve_bit", 1);
  br->sym("monochrome", 1);
  br->sym("chroma_subsampling_x", 1);
  br->sym("chroma_subsampling_y", 1);
  br->sym("chroma_sample_position", 2);
  br->sym("reserved", 3);

  auto initial_presentation_delay_present = br->sym("initial_presentation_delay_present", 1);

  if(initial_presentation_delay_present)
  {
    br->sym("initial_presentation_delay_minus_one", 4);
  }
  else
  {
    br->sym("reserved", 4);
  }

  br->sym("configOBUs", 0);

  Av1State state;

  while(!br->empty())
    parseAv1Obus(br, state, true);
}

