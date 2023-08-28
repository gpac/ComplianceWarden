#include "get_data.h"

#include <sstream>

std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc);

std::string sampleValues::pretty() const
{
  std::stringstream res;
  res << size << " @ " << offset << " [" << (void *)position << "]";
  return res.str();
}
BitReader sampleValues::getSample() const
{
  if(position == nullptr) {
    return { nullptr, 0 };
  }
  return { position, size };
}

std::vector<int> getOffsets(const Box *offsetBox)
{
  if(!offsetBox) {
    return {};
  }

  std::vector<int> res;

  for(auto &sym : offsetBox->syms) {
    if(!strcmp(sym.name, "chunk_offset"))
      res.push_back(sym.value);
  }

  return res;
}

std::vector<int> getSizes(const Box *sizeBox)
{
  if(!sizeBox) {
    return {};
  }

  std::vector<int> res;

  int64_t sampleSize = 0;
  int64_t sampleCount = 0;
  for(auto &sym : sizeBox->syms) {
    if(!strcmp(sym.name, "sample_size")) {
      sampleSize = sym.value;
    }
    if(!strcmp(sym.name, "sample_count")) {
      sampleCount = sym.value;
    }
    if(!strcmp(sym.name, "entry_size")) {
      res.push_back(sym.value);
    }
  }
  if(res.empty()) {
    for(auto i = 0; i < sampleCount; i++) {
      res.push_back(sampleSize);
    }
  }

  return res;
}

std::vector<sampleValues> getSampleValues(const Box &root, IReport *out, const Box *offsetBox, const Box *sizeBox)
{

  auto offsets = getOffsets(offsetBox);
  auto sizes = getSizes(sizeBox);

  if(offsets.size() != sizes.size()) {
    out->error(
      "Found %llu samples offsets, and %llu sample sizes, while expecting an equal amount", offsets.size(),
      sizes.size());
    return {};
  }

  std::vector<sampleValues> res;
  for(size_t i = 0; i < sizes.size(); i++) {
    if(i < offsets.size()) {
      res.push_back({ offsets[i], sizes[i], root.original + offsets[i] });
    } else {
      res.push_back({ 0, sizes[i], nullptr });
    }
  }

  return res;
}

const Box *selectEither(const Box &root, IReport *out, uint32_t option1, uint32_t option2)
{
  auto fourCC1 = toString(option1);
  auto fourCC2 = toString(option2);
  auto boxList1 = findBoxes(root, option1);
  auto boxList2 = findBoxes(root, option2);
  if(boxList1.size() > 1) {
    out->error("%llu '%s' boxes found, when at most 1 is expected", boxList1.size(), fourCC1.c_str());
    return nullptr;
  }
  if(boxList2.size() > 1) {
    out->error("%llu '%s' boxes found, when at most 1 isexpected", boxList2.size(), fourCC2.c_str());
    return nullptr;
  }
  if((boxList1.size() + boxList2.size()) != 1) {
    out->error(
      "%llu '%s'/'%s' boxes found, when exactly 1 is expected", boxList1.size() + boxList2.size(), fourCC1.c_str(),
      fourCC2.c_str());
    return nullptr;
  }
  return boxList1.size() ? boxList1[0] : boxList2[0];
}

std::vector<sampleValues> getData(const Box &root, IReport *out, uint64_t trackId)
{
  auto trakBoxes = findBoxes(root, FOURCC("trak"));
  for(auto &trakBox : trakBoxes) {

    auto tkhdBoxes = findBoxes(*trakBox, FOURCC("tkhd"));
    if(tkhdBoxes.size() != 1) {
      out->error("%llu 'tkhd' boxes found, when 1 is expected", tkhdBoxes.size());
      return {};
    }

    uint32_t thisTrackId = 0;
    for(auto &sym : tkhdBoxes[0]->syms)
      if(!strcmp(sym.name, "track_ID"))
        thisTrackId = sym.value;

    if(thisTrackId != trackId) {
      continue;
    }

    auto offsetBox = selectEither(*trakBox, out, FOURCC("stco"), FOURCC("co64"));
    auto sizeBox = selectEither(*trakBox, out, FOURCC("stsz"), FOURCC("stz2"));

    if(!offsetBox || !sizeBox) {
      return {};
    }

    return getSampleValues(root, out, offsetBox, sizeBox);
  }
  return {};
}
