#include "av1_utils.h"
#include "box_reader_impl.h"

struct sampleValues {
  int64_t offset;
  uint64_t size;
  uint8_t *position;
  std::string pretty() const;
  BitReader getSample() const;
};

std::vector<sampleValues> getData(const Box &root, IReport *out, uint64_t trackId);
