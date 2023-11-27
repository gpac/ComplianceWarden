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
  return { position, (int)size };
}

namespace
{
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

std::vector<uint64_t> getSizes(const Box *sizeBox)
{
  if(!sizeBox) {
    return {};
  }

  std::vector<uint64_t> res;

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

std::vector<sampleFlags> getFlags(const Box *flagsBox)
{
  if(!flagsBox) {
    return {};
  }

  std::vector<sampleFlags> res;

  sampleFlags sf = {};
  for(auto &sym : flagsBox->syms) {
    if(!strcmp(sym.name, "is_leading")) {
      sf = {};
      sf.isLeading = sym.value;
    }
    if(!strcmp(sym.name, "sample_depends_on")) {
      sf.sampleDependsOn = sym.value;
    }
    if(!strcmp(sym.name, "sample_is_depended_on")) {
      sf.sampleIsDependedOn = sym.value;
    }
    if(!strcmp(sym.name, "sample_has_redundancy")) {
      sf.sampleHasRedundancy = sym.value;
      res.push_back(sf);
    }
  }

  return res;
}

std::vector<sampleValues>
getSampleValues(const Box &root, IReport *out, const Box *offsetBox, const Box *sizeBox, const Box *flagsBox)
{

  auto offsets = getOffsets(offsetBox);
  auto sizes = getSizes(sizeBox);
  auto flags = getFlags(flagsBox);

  if(offsets.size() != sizes.size()) {
    out->error(
      "Found %llu samples offsets, and %llu sample sizes, while expecting an equal amount", offsets.size(),
      sizes.size());
    return {};
  }

  if(flags.size() != 0 && flags.size() != sizes.size()) {
    out->error(
      "Found %llu sample flags, and %llu sample sizes, while expecting an equal amount or no flags at all",
      flags.size(), sizes.size());
    return {};
  }

  std::vector<sampleValues> res;
  for(size_t i = 0; i < sizes.size(); i++) {
    if(offsets[i] + sizes[i] > root.size) {
      out->error(
        "Sample %zu of %zu bytes @ offset %zu exceeds root size of %zu bytes", i, sizes[i], offsets[i], root.size);
      return {};
    }
    sampleValues sv = { offsets[i], sizes[i], root.original + offsets[i] };

    if(flags.size() > 0) {
      sv.flags = flags[i];
    }

    res.push_back(sv);
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
}

std::vector<sampleValues> getFragmentedData(const Box &root, IReport *out, uint64_t trackId)
{
  std::vector<sampleValues> res;

  // Get all movie fragments
  auto moofBoxes = findBoxes(root, FOURCC("moof"));
  for(auto &moofBox : moofBoxes) {

    // Get all track fragments
    auto trafBoxes = findBoxes(*moofBox, FOURCC("traf"));
    for(size_t trafIndex = 0; trafIndex < trafBoxes.size(); trafIndex++) {
      auto trafBox = trafBoxes[trafIndex];

      // Get track fragment header
      auto tfhdBoxes = findBoxes(*trafBox, FOURCC("tfhd"));
      if(tfhdBoxes.size() != 1) {
        out->error("%llu 'tfhd' boxes found, when 1 is expected", tfhdBoxes.size());
        return {};
      }

      uint32_t thisTrackId = 0;
      int64_t baseDataOffset = 0;
      uint64_t defaultSampleSize = 0;
      uint32_t defaultSampleFlags = 0;

      for(auto &sym : tfhdBoxes[0]->syms) {
        if(!strcmp(sym.name, "track_ID"))
          thisTrackId = sym.value;
        if(!strcmp(sym.name, "base_data_offset"))
          baseDataOffset = sym.value;
        if(!strcmp(sym.name, "default_sample_size"))
          defaultSampleSize = sym.value;
        if(!strcmp(sym.name, "default_sample_flags"))
          defaultSampleFlags = sym.value;
      }

      if(thisTrackId != trackId) {
        continue;
      }

      // State for this track fragment
      int64_t dataOffset = trafIndex == 0 ? baseDataOffset : -1;

      // Get track run boxes
      auto trunBoxes = findBoxes(*trafBox, FOURCC("trun"));
      for(auto &trunBox : trunBoxes) {
        uint32_t sampleCount = 0;
        int64_t dataOffsetTrun = -1;
        uint32_t flags = 0;
        for(auto &sym : trunBox->syms) {
          if(!strcmp(sym.name, "sample_count"))
            sampleCount = sym.value;
          if(!strcmp(sym.name, "data_offset"))
            dataOffsetTrun = sym.value;
          if(!strcmp(sym.name, "flags"))
            flags = sym.value;
        }

        // If data_offset is present, it is relative to base_data_offset
        if(dataOffsetTrun != -1)
          dataOffsetTrun += baseDataOffset;

        // If data_offset is not present, it is relative to previous run or base_data_offset (if first traf)
        if(dataOffsetTrun == -1)
          dataOffsetTrun = dataOffset;

        if(dataOffsetTrun == -1) {
          out->error("Could not determine data offset");
          return {};
        }

        // Check if first-sample-flags-present and sample-flags-present are set
        if(flags & 0x000004 && flags & 0x000400) {
          out->error("Both first-sample-flags-present and sample-flags-present are set");
          return {};
        }

        // Get offsets, sizes, flags
        std::vector<int> s_offsets;
        std::vector<uint64_t> s_sizes;
        std::vector<uint32_t> s_flags;

        // Get sizes
        if(flags & 0x000200) {
          // Each sample has its own size
          for(auto &sym : trunBox->syms) {
            if(!strcmp(sym.name, "sample_size"))
              s_sizes.push_back(sym.value);
          }
        } else {
          // All samples have the same size
          if(defaultSampleSize == 0) {
            out->error("Could not determine sample size");
            return {};
          }
          for(uint32_t i = 0; i < sampleCount; i++)
            s_sizes.push_back(defaultSampleSize);
        }

        // Get sample flags
        if(flags & 0x000004) {
          // This overrides the first sample flags
          for(auto &sym : trunBox->syms) {
            if(!strcmp(sym.name, "first_sample_flags"))
              s_flags.push_back(sym.value);
          }
          for(uint32_t i = 1; i < sampleCount; i++)
            s_flags.push_back(defaultSampleFlags);
        } else if(flags & 0x000400) {
          // Each sample has its own flags
          for(auto &sym : trunBox->syms) {
            if(!strcmp(sym.name, "sample_flags"))
              s_flags.push_back(sym.value);
          }
        } else {
          // All samples have the default flags
          for(uint32_t i = 0; i < sampleCount; i++)
            s_flags.push_back(defaultSampleFlags);
        }

        // Get offsets
        for(uint32_t i = 0; i < sampleCount; i++) {
          s_offsets.push_back(dataOffsetTrun);
          dataOffsetTrun += s_sizes[i];
        }

        // Add samples to result
        for(uint32_t i = 0; i < sampleCount; i++) {
          if(s_offsets[i] + s_sizes[i] > root.size) {
            out->error(
              "Sample %zu of %zu bytes @ offset %zu exceeds root size of %zu bytes", i + 1, s_sizes[i], s_offsets[i],
              root.size);
            return {};
          }
          sampleFlags sf = {};
          memcpy(&sf, &s_flags[i], sizeof(sf));
          res.push_back({ s_offsets[i], s_sizes[i], root.original + s_offsets[i], sf });
        }

        // Update data offset for next trun
        dataOffset = dataOffsetTrun;
      }
    }
  }
  return res;
}

std::vector<sampleValues> getNonFragmentedData(const Box &root, IReport *out, uint64_t trackId)
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

    const Box *sdtpBox = nullptr;
    auto sdtpBoxes = findBoxes(*trakBox, FOURCC("sdtp"));
    if(!sdtpBoxes.empty()) {
      sdtpBox = sdtpBoxes[0];
    }

    if(!offsetBox || !sizeBox) {
      return {};
    }

    return getSampleValues(root, out, offsetBox, sizeBox, sdtpBox);
  }
  return {};
}

std::vector<sampleValues> getData(const Box &root, IReport *out, uint64_t trackId)
{
  bool fragmented = false;

  // Check if mvex box is present
  auto mvexBoxes = findBoxes(root, FOURCC("mvex"));
  if(!mvexBoxes.empty()) {
    fragmented = true;
  }

  // Check if moof box is present
  auto moofBoxes = findBoxes(root, FOURCC("moof"));
  if(!moofBoxes.empty() && !fragmented) {
    fragmented = true;
    out->warning("Fragmented file detected but no 'mvex' box is present to signal it");
  }

  if(fragmented)
    return getFragmentedData(root, out, trackId);
  else
    return getNonFragmentedData(root, out, trackId);
}
