#include "spec.h"
#include "fourcc.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>

bool isVisualSampleEntry(uint32_t fourcc);

static const SpecDesc specIsobmff =
{
  "isobmff",
  "ISO Base Media File Format\n"
  "MPEG-4 part 12 - ISO/IEC 14496-12 - MPEG-4 Part 12 - m17277 (6th+FDAM1+FDAM2+COR1-R4)",
  {},
  {
    {
      "Section: 12.1.3.2\n"
      "CleanApertureBox 'clap' and PixelAspectRatioBox 'pasp' in VisualSampleEntry",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> FourCCs;
        std::vector<const Box*> found;

        auto checkIntegrityPasp = [&] (const Box& pasp) {
            if(pasp.size != 16)
              out->error("'pasp' box size is %d bytes (expected 16)", pasp.size);

            for(auto& field : pasp.syms)
              if(strcmp(field.name, "size") && strcmp(field.name, "fourcc")
                 && strcmp(field.name, "hSpacing") && strcmp(field.name, "vSpacing"))
                out->error("Invalid 'pasp' field \"%s\" (value=%lld)", field.name, field.value);
          };

        auto checkIntegrityClap = [&] (const Box& clap) {
            if(clap.size != 40)
              out->error("'clap' box size is %d bytes (expected 40)", clap.size);
          };

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            for(auto& moovChild : box.children)
              if(moovChild.fourcc == FOURCC("trak"))
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("mdia"))
                    for(auto& mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto& minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl"))
                            for(auto& stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto& stsdChild : stblChild.children)
                                  if(isVisualSampleEntry(stsdChild.fourcc))
                                    for(auto& sampleEntryChild : stsdChild.children)
                                      if(sampleEntryChild.fourcc == FOURCC("clap") || sampleEntryChild.fourcc == FOURCC("pasp"))
                                      {
                                        FourCCs.push_back(sampleEntryChild.fourcc);
                                        found.push_back(&sampleEntryChild);
                                      }

        auto expected = std::vector<uint32_t>({ FOURCC("clap"), FOURCC("pasp") });

        for(size_t i = 0; i < FourCCs.size(); ++i)
        {
          if(FourCCs[i] != expected[i])
          {
            out->error("Expecting: 'clap', 'pasp', got:");

            for(auto f : FourCCs)
              out->error("\t'%s'", toString(f).c_str());
          }
        }

        // Look for other invalidly positioned 'clap' and 'pasp' boxes
        auto findBox = [&] (const Box* box) {
            for(auto b : found)
              if(box == b)
                return true;

            return false;
          };

        std::function<void(const Box &, const uint32_t, std::function<void(const Box &)> )> parse =
          [&] (const Box& parent, const uint32_t fourCC, std::function<void(const Box &)> checkIntegrity)
          {
            for(auto& box : parent.children)
              if(box.fourcc == fourCC)
              {
                if(!findBox(&box))
                {
                  if(parent.fourcc != FOURCC("ipco")) /*ipco is also a valid parent*/
                  {
                    checkIntegrity(box);
                    out->warning("Unexpected '%s' position (parent is '%s')", toString(fourCC).c_str(), toString(parent.fourcc).c_str());
                  }
                }
              }
              else
                parse(box, fourCC, checkIntegrity);
          };

        parse(root, FOURCC("clap"), checkIntegrityClap);
        parse(root, FOURCC("pasp"), checkIntegrityPasp);
      }
    },
    {
      "Section: 8.11.14.1\n"
      "Each ItemPropertyAssociationBox shall be ordered by increasing item_ID,\n"
      "and there shall be at most one occurrence of a given item_ID, in the set of\n"
      "ItemPropertyAssociationBox boxes",
      [] (Box const& root, IReport* out)
      {
        std::vector<std::vector<uint32_t>> itemIds;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    itemIds.resize(itemIds.size() + 1);

                    for(auto& sym : iprpChild.syms)
                      if(!strcmp(sym.name, "item_ID"))
                        itemIds.back().push_back(sym.value);
                  }

        for(auto& ipma : itemIds)
        {
          uint32_t lastItemId = 0;

          for(auto itemId : ipma)
          {
            if(itemId <= lastItemId)
              out->error("Each ItemPropertyAssociationBox shall be ordered by increasing item_ID (%u, last=%u)", itemId, lastItemId);

            lastItemId = itemId;
          }
        }

        std::vector<uint32_t> itemIdsCount;

        for(auto& ipma : itemIds)
          for(auto itemId : ipma)
            if(std::find(itemIdsCount.begin(), itemIdsCount.end(), itemId) == itemIdsCount.end())
              itemIdsCount.push_back(itemId);
            else
              out->error("There shall be at most one occurrence of a given item_ID but %u found several times", itemId);
      }
    },
    {
      "Section: 8.11.14.1\n"
      "There shall be at most one ItemPropertyAssociationBox with a given pair of\n"
      "values of version and flags",
      [] (Box const& root, IReport* out)
      {
        std::vector<std::pair<int64_t, int64_t>> ipmas;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    int64_t version = 0;

                    for(auto& sym : iprpChild.syms)
                      if(!strcmp(sym.name, "version"))
                        version = sym.value;
                      else if(!strcmp(sym.name, "flags"))
                      {
                        if(std::find(ipmas.begin(), ipmas.end(), std::pair<int64_t, int64_t> { version, sym.value }) == ipmas.end())
                          ipmas.push_back({ version, sym.value });
                        else
                          out->error("There shall be at most one ipma with a given pair of values of version and flags but { version=%lld, flags=%lld } found several times", version, sym.value);

                        break;
                      }
                  }
      }
    }
  },
  nullptr,
};

static auto const registered = registerSpec(&specIsobmff);

