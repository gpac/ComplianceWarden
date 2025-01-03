#pragma once

#include "core/box_reader_impl.h"
#include "av1_utils.h"

struct sampleFlags {
  uint32_t reserved : 4;
  uint32_t isLeading : 2;
  uint32_t sampleDependsOn : 2;
  uint32_t sampleIsDependedOn : 2;
  uint32_t sampleHasRedundancy : 2;
  uint32_t samplePaddingValue : 3;
  uint32_t sampleIsDifferenceSample : 1;
  uint32_t sampleDegradationPriority : 16;
};

struct sampleValues {
  int64_t offset;
  uint64_t size;
  uint8_t *position;
  sampleFlags flags = {};
  std::string pretty() const;
  BitReader getSample() const;
};

std::vector<sampleValues> getData(const Box &root, IReport *out, uint64_t trackId);
