#pragma once

#include "box_reader.h"
#include <string>

enum
{
  OBU_SEQUENCE_HEADER = 1,
  OBU_FRAME_HEADER = 3,
  OBU_METADATA = 5,
  OBU_FRAME = 6,
  OBU_REDUNDANT_FRAME_HEADER = 7,
};

auto const AV1_KEY_FRAME = 0;

struct IReader;
struct Box;

struct AV1CodecConfigurationRecord
{
  int64_t seq_profile;
  int64_t seq_level_idx_0;
  int64_t seq_tier_0;
  int64_t high_bitdepth;
  int64_t twelve_bit;
  int64_t mono_chrome;
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
  int64_t color_range = 0;
  AV1CodecConfigurationRecord av1c {};
};

int64_t parseAv1Obus(IReader* br, av1State& state, bool storeUnparsed);

