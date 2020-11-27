#include "spec.h"
#include "box_reader_impl.h"
#include <algorithm> // std::find
#include <cassert>
#include <cstring> // strcmp
#include <map>
#include <memory> // make_unique
#include <stdexcept>

void checkEssential(Box const& root, IReport* out, uint32_t fourcc);
std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);

namespace
{
bool operator == (const Symbol& lhs, const Symbol& rhs)
{
  if(lhs.numBits != rhs.numBits)
    return false;

  if(lhs.value != rhs.value)
    return false;

  if(strcmp(lhs.name, rhs.name))
    return false;

  return true;
}

bool operator == (const std::vector<Symbol>& lhs, const std::vector<Symbol>& rhs)
{
  if(lhs.size() != rhs.size())
    return false;

  for(size_t i = 0; i < lhs.size(); ++i)
    if(!(lhs[i] == rhs[i]))
      return false;

  return true;
}

struct AV1CodecConfigurationRecord
{
  int64_t seq_profile;
  int64_t seq_level_idx_0;
  int64_t seq_tier_0;
  int64_t high_bitdepth;
  int64_t twelve_bit;
  int64_t mono_chrome;
  int64_t color_range;
  int64_t chroma_subsampling_x;
  int64_t chroma_subsampling_y;
  int64_t chroma_sample_position;
  // initial_presentation_delay
  // configOBUs[]

  std::string toString()
  {
    return std::string("\t\tseq_profile=") + std::to_string(seq_profile) + "\n" +
           "\t\tseq_level_idx_0=" + std::to_string(seq_level_idx_0) + "\n" +
           "\t\tseq_tier_0=" + std::to_string(seq_tier_0) + "\n" +
           "\t\thigh_bitdepth=" + std::to_string(high_bitdepth) + "\n" +
           "\t\ttwelve_bit=" + std::to_string(twelve_bit) + "\n" +
           "\t\tmono_chrome=" + std::to_string(mono_chrome) + "\n" +
           "\t\tchroma_subsampling_x=" + std::to_string(chroma_subsampling_x) + "\n" +
           "\t\tchroma_subsampling_y=" + std::to_string(chroma_subsampling_y) + "\n" +
           "\t\tchroma_sample_position=" + std::to_string(chroma_sample_position);
  }
};

struct av1State
{
  bool reduced_still_picture_header = false;
  bool frame_id_numbers_present_flag = false;
  int64_t delta_frame_id_length_minus_2 = 0;
  int64_t additional_frame_id_length_minus_1 = 0;
  AV1CodecConfigurationRecord av1c {};
};

auto const KEY_FRAME = 0;

#define READ_UNTIL_NEXT_BYTE(readBits) \
  if(readBits % 8){ \
    auto remainderBits = 8 - (readBits % 8); \
    br->sym("bits", remainderBits); \
    readBits += remainderBits; \
  }

struct ReaderBits
{
  ReaderBits(IReader* delegate) : delegate(delegate) {}

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

void parseAv1ColorConfig(ReaderBits* br, int64_t seq_profile, AV1CodecConfigurationRecord& av1c)
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
    av1c.color_range = br->sym("color_range", 1);
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
    av1c.color_range = 1;
    av1c.chroma_subsampling_x = 0;
    av1c.chroma_subsampling_y = 0;
  }
  else
  {
    av1c.color_range = br->sym("color_range", 1);

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
      br->sym("chroma_sample_position", 1);
    }
  }

  br->sym("separate_uv_delta_q", 1);
}

int parseAv1SeqHdr(IReader* reader, av1State& state)
{
  auto br = std::make_unique<ReaderBits>(reader);

  state.av1c.seq_profile = br->sym("seq_profile", 3);
  br->sym("still_picture", 1);
  state.reduced_still_picture_header = br->sym("reduced_still_picture_header", 1);

  if(state.reduced_still_picture_header)
  {
    br->sym("timing_info_present_flag", 0); // =0
    auto decoder_model_info_present_flag = br->sym("decoder_model_info_present_flag", 0); // =0
    assert(decoder_model_info_present_flag == 0);
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
    assert(timing_info_present_flag == 0); // timing info and consequence in uncompressed header not implemented

    auto initial_display_delay_present_flag = br->sym("initial_display_delay_present_flag", 1);
    assert(initial_display_delay_present_flag == 0);

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
      br->sym(" enable_jnt_comp", 1);
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
  parseAv1ColorConfig(br.get(), state.av1c.seq_profile, state.av1c);
  br->sym("film_grain_params_present", 1);

  auto readBits = br->count;
  READ_UNTIL_NEXT_BYTE(readBits);
  return readBits / 8;
}

int parseAv1UncompressedHeader(IReader* reader, av1State const& state)
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
      auto frame_to_show_map_idx = br->sym("frame_to_show_map_idx", 3);

      // Not covered: there is an assert in the sequence header.
      // if ( decoder_model_info_present_flag && !equal_picture_interval ) {
      // temporal_point_info( )
      // }
      // refresh_frame_flags = 0

      if(state.frame_id_numbers_present_flag)
      {
        br->sym("display_frame_id", idLen);
      }

      assert(frame_to_show_map_idx == 0);
      assert(0); // we don't refresh RefFrameType
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

enum
{
  OBU_SEQUENCE_HEADER = 1,
  OBU_FRAME_HEADER = 3,
  OBU_METADATA = 5,
  OBU_FRAME = 6,
  OBU_REDUNDANT_FRAME_HEADER = 7,
};

void parseAv1Obus(IReader* br, av1State& state)
{
  br->sym("obu", 0); // virtual OBU separator
  br->sym("forbidden", 1);
  auto obu_type = br->sym("obu_type", 4);
  auto obu_extension_flag = br->sym("obu_extension_flag", 1);
  auto obu_has_size_field = br->sym("obu_has_size_field", 1);

  if(!obu_has_size_field)
    assert(0 && "obu_has_size_field shall be set");

  br->sym("obu_reserved_1bit", 1);

  if(obu_extension_flag)
  {
    br->sym("temporal_id", 3);
    br->sym("spatial_id", 2);
    br->sym("extension_header_reserved_3bits", 3);
  }

  auto leb128_read = [] (IReader* br, int* bytes) -> uint64_t {
      uint64_t value = 0;
      uint8_t Leb128Bytes = 0;

      for(int i = 0; i < 8; i++)
      {
        uint8_t leb128_byte = br->sym("leb128_byte", 8);
        value |= (((uint64_t)(leb128_byte & 0x7f)) << (i * 7));
        Leb128Bytes += 1;

        if(!(leb128_byte & 0x80))
          break;
      }

      if(bytes)
        *bytes = Leb128Bytes;

      return value;
    };

  int leb128Bytes = 0;
  long long unsigned obuSize = leb128_read(br, &leb128Bytes);
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
    break;
  default: break;
  }

  while(obuSize-- > 0)
  {
    if(br->empty())
    {
      fprintf(stderr, "Incomplete OBU (remaining to read=%llu)\n", obuSize + 1);
      break;
    }

    br->sym("byte", 8);
  }
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

  av1State state;

  while(!br->empty())
  {
    parseAv1Obus(br, state);
  }
}

ParseBoxFunc* getParseFunctionAvif(uint32_t fourcc)
{
  switch(fourcc)
  {
  case FOURCC("av1C"):
    return &parseAv1C;
  default:
    return nullptr;
  }
}

std::vector<uint32_t /*itemId*/> findAv1ImageItems(Box const& root)
{
  std::vector<uint32_t> av1ImageItemIDs;

  // Find AV1 Image Items
  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iinf"))
          for(auto& iinfChild : metaChild.children)
            if(iinfChild.fourcc == FOURCC("infe"))
            {
              uint32_t itemId = 0;

              for(auto& sym : iinfChild.syms)
              {
                if(!strcmp(sym.name, "item_ID"))
                  itemId = sym.value;
                else if(!strcmp(sym.name, "item_type"))
                  if(sym.value == FOURCC("av01"))
                    av1ImageItemIDs.push_back(itemId);
              }
            }

  return av1ImageItemIDs;
}

struct ItemLocation
{
  int construction_method = 0, data_reference_index = 0;
  int64_t base_offset = 0;
  struct Span
  {
    int64_t offset = 0, length = 0;
  };
  std::vector<Span> extents;

  int computeOffset(IReport* out)
  {
    if(construction_method > 1 || extents.size() > 1)
    {
      out->warning("construction_method > 1 not supported");
      return 0;
    }

    if(extents.size() > 1)
    {
      out->warning("iloc with several extensions not supported");
      return 0;
    }

    if(data_reference_index > 0)
    {
      out->warning("data_reference_index > 0 not supported");
      return 0;
    }

    int originOffset = base_offset;

    if(!extents.empty())
      originOffset += extents[0].offset;

    return originOffset;
  }
};

std::vector<ItemLocation::Span> getAv1ImageItemsDataOffsets(Box const& root, IReport* out, uint32_t itemID)
{
  std::vector<ItemLocation::Span> spans;

  std::vector<ItemLocation> itemLocs;

  for(auto& box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto& metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iloc"))
        {
          int64_t currOffset = 0;

          for(auto& sym : metaChild.syms)
          {
            if(!strcmp(sym.name, "item_ID"))
            {
              if(sym.value != itemID)
                break;

              itemLocs.resize(itemLocs.size() + 1);

              continue;
            }

            if(itemLocs.empty())
              continue;

            auto& itemLoc = itemLocs.back();

            if(!strcmp(sym.name, "construction_method"))
              itemLoc.construction_method = sym.value;

            if(!strcmp(sym.name, "data_reference_index"))
              itemLoc.data_reference_index = sym.value;

            if(!strcmp(sym.name, "base_offset"))
              itemLoc.base_offset = sym.value;

            if(!strcmp(sym.name, "extent_offset"))
              currOffset = sym.value;

            if(!strcmp(sym.name, "extent_length"))
              itemLoc.extents.push_back({ currOffset, sym.value });
          }
        }

  for(auto& itemLoc : itemLocs)
    spans.push_back({ itemLoc.computeOffset(out), itemLoc.extents[0].length }); // we assume no idat-based check (construction_method = 1)

  return spans;
}

Box const& explore(Box const& root, uint64_t targetOffset)
{
  for(auto& box : root.children)
    if(box.position + box.size > targetOffset)
      return explore(box, targetOffset);

  return root;
}

std::vector<uint8_t> getAV1ImageItemData(Box const& root, IReport* out, uint32_t itemId)
{
  std::vector<uint8_t> bytes;
  auto spans = getAv1ImageItemsDataOffsets(root, out, itemId);

  for(auto span : spans)
  {
    auto box = explore(root, span.offset);
    int64_t diffBits = 8 * (span.offset - box.position);

    bool firstTime = true;

    for(auto sym : box.syms)
    {
      if(diffBits > 0)
      {
        diffBits -= sym.numBits;
        continue;
      }

      if(firstTime)
      {
        if(diffBits && diffBits + sym.numBits)
          out->warning("Could not locate AV1 Image Item Data (diffBits=%s)", toString(diffBits).c_str());

        firstTime = false;
      }

      if(strlen(sym.name) || sym.numBits != 8)
      {
        out->warning("Wrong symbol detected (name=%s, size=%d bits). Stopping import.", sym.name, sym.numBits);
        break;
      }

      bytes.push_back((uint8_t)sym.value);

      if((int64_t)bytes.size() >= span.length)
        break;
    }
  }

  return bytes;
}
} // namespace

static const SpecDesc specAvif =
{
  "avif",
  "AVIF v1.0.0, 19 February 2019\n"
  "https://aomediacodec.github.io/av1-avif/",
  { "heif" },
  {
    {
      "Section 2.1 AV1 Image Item\n"
      "The AV1 Image Item shall be associated with an AV1 Item Configuration Property.",
      [] (Box const& root, IReport* out)
      {
        auto av1ImageItemIDs = findAv1ImageItems(root);

        // AV1 Image Item shall be associated with an AV1 Item Configuration Property
        std::vector<uint32_t> av1cPropertyIndex; /* 1-based */

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipco"))
                    for(uint32_t i = 1; i <= iprpChild.children.size(); ++i)
                      if(iprpChild.children[i - 1].fourcc == FOURCC("av1C"))
                        av1cPropertyIndex.push_back(i);

        std::map<uint32_t /*itemId*/, bool> av1ImageItemIDsAssociated;

        for(auto itemId : av1ImageItemIDs)
          av1ImageItemIDsAssociated.insert({ itemId, false });

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    uint32_t itemId = 0;

                    for(auto& sym : iprpChild.syms)
                    {
                      if(!strcmp(sym.name, "item_ID"))
                        itemId = sym.value;
                      else if(!strcmp(sym.name, "property_index"))
                        if(std::find(av1cPropertyIndex.begin(), av1cPropertyIndex.end(), sym.value) != av1cPropertyIndex.end())
                          av1ImageItemIDsAssociated[itemId] = true;
                    }
                  }

        for(auto& item : av1ImageItemIDsAssociated)
          if(item.second == false)
            out->error("AV1 Image Item (ID=%u) shall be associated with an AV1 Item Configuration Property", item.first);
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data shall be identical to the content of an AV1 Sample marked as sync",
      [] (Box const& root, IReport* out)
      {
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State stateUnused;

          while(!br.empty())
            parseAv1Obus(&br, stateUnused);

          /* we know we've seen the sequence header (av1C) and the frame header*/
          bool showFrame = false, keyFrame = false;
          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
          {
            if(!strcmp(sym.name, "show_frame") && (!sym.numBits || sym.value))
              showFrame = true;

            if(!strcmp(sym.name, "key_frame"))
              keyFrame = true;

            if(!strcmp(sym.name, "frame_type"))
              if(sym.value == KEY_FRAME)
                keyFrame = true;
          }

          if(!(showFrame && keyFrame))
            out->error("AV1 Sample shall be marked as sync (showFrame=%d, keyFrame=%d)", (int)showFrame, (int)keyFrame);
        }
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data shall have exactly one Sequence Header OBU.",
      [] (Box const& root, IReport* out)
      {
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        int seqHdrNum = 0;

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State stateUnused;

          while(!br.empty())
            parseAv1Obus(&br, stateUnused);

          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
            if(!strcmp(sym.name, "seqhdr"))
              seqHdrNum++;
        }

        if(seqHdrNum != 1)
          out->error("Expected one sequence Header OBU but found %d", seqHdrNum);
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data should have its still_picture flag set to 1.",
      [] (Box const& root, IReport* out)
      {
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State stateUnused;

          while(!br.empty())
            parseAv1Obus(&br, stateUnused);

          assert(br.myBox.children.empty());

          for(auto& sym : br.myBox.syms)
            if(!strcmp(sym.name, "still_picture"))
              if(sym.value == 0)
                out->warning("still_picture flag set to 0.");
        }
      }
    },
    {
      "Section 2.1\n"
      "The AV1 Image Item Data should have its reduced_still_picture_header flag set to 1.",
      [] (Box const& root, IReport* out)
      {
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State state;

          while(!br.empty())
            parseAv1Obus(&br, state);

          if(!state.reduced_still_picture_header)
            out->warning("reduced_still_picture_header flag set to 0.");
        }
      }
    },
    {
      "Section 2.1\n"
      "Sequence Header OBUs should not be present in the AV1CodecConfigurationBox.",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
          {
            auto av1Cs = findBoxes(box, FOURCC("av1C"));

            for(auto& av1C : av1Cs)
              for(auto& sym : av1C->syms)
                if(!strcmp(sym.name, "seqhdr"))
                  out->warning("Sequence Header OBUs should not be present in the AV1CodecConfigurationBox.");
          }
        }
      }
    },
    {
      "Section 2.2.1\n"
      "If a Sequence Header OBU is present in the AV1CodecConfigurationBox, it shall match the\n"
      "Sequence Header OBU in the AV1 Image Item Data.",
      [] (Box const& root, IReport* out)
      {
        std::vector<Symbol> av1cSymbols, av1ImageItemDataSeqHdr;

        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
          {
            auto av1Cs = findBoxes(box, FOURCC("av1C"));

            if(av1Cs.size() > 1)
              out->warning("Several av1C found. Please contact us to provide a sample!");

            for(auto& av1C : av1Cs)
            {
              bool seqHdrFound = false;

              for(auto& sym : av1C->syms)
              {
                if(!strcmp(sym.name, "seqhdr"))
                  seqHdrFound = true;

                if(!strcmp(sym.name, "/seqhdr"))
                  seqHdrFound = false;

                if(seqHdrFound)
                  av1cSymbols.push_back(sym);
              }
            }
          }
        }

        if(av1cSymbols.empty())
          return;

        auto const av1ImageItemIDs = findAv1ImageItems(root);

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State state;

          while(!br.empty())
            parseAv1Obus(&br, state);

          bool seqHdrFound = false;

          for(auto& sym : br.myBox.syms)
          {
            if(!strcmp(sym.name, "seqhdr"))
              seqHdrFound = true;

            if(!strcmp(sym.name, "/seqhdr"))
              seqHdrFound = false;

            if(seqHdrFound)
              av1ImageItemDataSeqHdr.push_back(sym);
          }
        }

        if(!(av1cSymbols == av1ImageItemDataSeqHdr))
          out->error("The Sequence Header OBU present in the AV1CodecConfigurationBox shall match the one in the AV1 Image Item Data.");
      }
    },
    {
      "Section 2.2.1\n"
      "The values of the fields in the AV1CodecConfigurationBox shall match those of the\n"
      "Sequence Header OBU in the AV1 Image Item Data.",
      [] (Box const& root, IReport* out)
      {
        AV1CodecConfigurationRecord av1cRef {};

        for(auto& box : root.children)
        {
          if(box.fourcc == FOURCC("meta"))
          {
            auto av1Cs = findBoxes(box, FOURCC("av1C"));

            if(av1Cs.size() > 1)
              out->warning("Several av1C found. Please contact us to provide a sample!");

            for(auto& av1C : av1Cs)
            {
              for(auto& sym : av1C->syms)
              {
                if(!strcmp(sym.name, "seq_profile"))
                  av1cRef.seq_profile = sym.value;

                if(!strcmp(sym.name, "seq_level_idx_0"))
                  av1cRef.seq_level_idx_0 = sym.value;

                if(!strcmp(sym.name, "seq_tier_0"))
                  av1cRef.seq_tier_0 = sym.value;

                if(!strcmp(sym.name, "high_bitdepth"))
                  av1cRef.high_bitdepth = sym.value;

                if(!strcmp(sym.name, "twelve_bit"))
                  av1cRef.twelve_bit = sym.value;

                if(!strcmp(sym.name, "monochrome"))
                  av1cRef.mono_chrome = sym.value;

                if(!strcmp(sym.name, "chroma_subsampling_x"))
                  av1cRef.chroma_subsampling_x = sym.value;

                if(!strcmp(sym.name, "chroma_subsampling_y"))
                  av1cRef.chroma_subsampling_y = sym.value;

                if(!strcmp(sym.name, "chroma_sample_position"))
                  av1cRef.chroma_sample_position = sym.value;
              }
            }
          }
        }

        auto const av1ImageItemIDs = findAv1ImageItems(root);

        for(auto itemId : av1ImageItemIDs)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State state;

          while(!br.empty())
            parseAv1Obus(&br, state);

          if(memcmp(&state.av1c, &av1cRef, sizeof(AV1CodecConfigurationRecord)))
            out->error("The values of the AV1CodecConfigurationBox shall match\n"
                       "the Sequence Header OBU in the AV1 Image Item Data:\n"
                       "\tAV1CodecConfigurationBox:\n%s\n"
                       "\tSequence Header OBU in the AV1 Image Item Data:\n%s\n",
                       state.av1c.toString().c_str(), av1cRef.toString().c_str());
        }
      }
    },
    {
      "Section 2.2.1\n"
      "Metadata OBUs, if present, shall match the values given in other item properties, such as\n"
      "the PixelInformationProperty or ColourInformationBox.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out; // TODO
      }
    },
    {
      "Section 2.2.1\n"
      "AV1 Item Configuration Property [...] shall be marked as essential.",
      [] (Box const& root, IReport* out)
      {
        checkEssential(root, out, FOURCC("av1C"));
      }
    },
    {
      "Section 3\n"
      "The track handler for an AV1 Image Sequence shall be 'pict'.",
      [] (Box const& root, IReport* out)
      {
        // This rule doesn't make sense as HEIF defines an Image Sequence as a track with
        // handler_type 'pict'. Let's test:
        // - when 'avis' brand is present ('should' be), there is at least one track with
        // 'pict' handler;
        // - and also contains a primary item that is an image item (here checked as av01
        // although this is not clearly specified).

        bool hasAvisBrand = false, hasOneTrackPict = false, hasPrimaryItemImage = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("ftyp"))
            for(auto& sym : box.syms)
              if(!strcmp(sym.name, "compatible_brand"))
                if(sym.value == FOURCC("avis"))
                  hasAvisBrand = true;

        uint32_t primaryItemId = -1;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("pitm"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "item_ID"))
                    primaryItemId = field.value;

        auto av1ImageItemIDs = findAv1ImageItems(root);

        if(std::find(av1ImageItemIDs.begin(), av1ImageItemIDs.end(), primaryItemId) != av1ImageItemIDs.end())
          hasPrimaryItemImage = true;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak"))
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto& mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("hdlr"))
                        for(auto& sym : mdiaChild.syms)
                          if(!strcmp(sym.name, "handler_type"))
                            if(sym.value == FOURCC("pict"))
                              hasOneTrackPict = true;

        {
          void (IReport::* report) (const char* fmt, ...);

          if(hasAvisBrand)
            report = &IReport::error;
          else
            report = &IReport::warning;

          if(hasOneTrackPict ^ hasPrimaryItemImage)
          {
            if(hasOneTrackPict)
              (out->*report)("Track with 'pict' handler found, but no primary item that is an AV1 image item found.");
            else if(hasAvisBrand)
              (out->*report)("Primary item that is an AV1 image item found, but no track with 'pict' handler found.");
          }
        }
      }
    },
    {
      "Section 4. Auxiliary Image Items\n"
      "The mono_chrome field in the Sequence Header OBU shall be set to 1.\n"
      "The color_range field in the Sequence Header OBU shall be set to 1.",
      [] (Box const& root, IReport* out)
      {
        // contains both image items and meta primary image item of the sequence
        auto const av1ImageItemIDs = findAv1ImageItems(root);

        std::vector<uint32_t> auxImages;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iref"))
                for(auto& field : metaChild.syms)
                {
                  if(!strcmp(field.name, "box_type"))
                    if(field.value != FOURCC("auxl"))
                      break;

                  if(!strcmp(field.name, "from_item_ID"))
                    if(std::find(av1ImageItemIDs.begin(), av1ImageItemIDs.end(), field.value) == av1ImageItemIDs.end())
                      break;

                  if(!strcmp(field.name, "to_item_ID"))
                    auxImages.push_back(field.value);
                }

        for(auto itemId : auxImages)
        {
          auto bytes = getAV1ImageItemData(root, out, itemId);

          BoxReader br;
          br.br = BitReader { bytes.data(), (int)bytes.size() };

          av1State state;

          while(!br.empty())
            parseAv1Obus(&br, state);

          assert(br.myBox.children.empty());

          if(!state.av1c.mono_chrome)
            out->error("The mono_chrome field in the Sequence Header OBU shall be set to 1 (item_ID=%u).", itemId);

          if(!state.av1c.color_range)
            out->error("The color_range field in the Sequence Header OBU shall be set to 1 (item_ID=%u).", itemId);
        }
      }
    },
  },
  getParseFunctionAvif,
};

static auto const registered = registerSpec(&specAvif);

