#include "spec.h"
#include "fourcc.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>

bool isVisualSampleEntry(uint32_t fourcc);
std::vector<const Box*> findBoxes(const Box& root, uint32_t fourcc);
std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> getItemDataOffsets(Box const& root, IReport* out, uint32_t itemID);

namespace
{
void findZeroSizedBoxes(Box const* const root, std::vector<const Box*>& zeroSizedBoxes)
{
  if(root->size == 0)
    zeroSizedBoxes.push_back(root);

  for(auto& box : root->children)
    findZeroSizedBoxes(&box, zeroSizedBoxes);
}

int arity(Box const& root, uint32_t fourcc)
{
  int counter = 0;

  for(auto& box : root.children)
    if(box.fourcc == fourcc)
      counter++;

  return counter;
};

const SpecDesc specIsobmff =
{
  "isobmff",
  "ISO Base Media File Format\n"
  "MPEG-4 part 12 - ISO/IEC 14496-12 - m17277 (6th+FDAM1+FDAM2+COR1-R4)",
  {},
  {
    {
      "Section 12.1.3.2\n"
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
      "Section 8.11.3: Item location box\n"
      "Data offset integrity: check we don't point out of the file",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> itemIds;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iloc"))
                for(auto& sym : metaChild.syms)
                  if(!strcmp(sym.name, "item_ID"))
                    itemIds.push_back(sym.value);

        const int64_t filesize = root.size;

        for(auto itemId : itemIds)
        {
          auto spans = getItemDataOffsets(root, out, itemId);

          for(auto& span : spans)
          {
            if(span.first + span.second > filesize)
              out->error("Data offset overflow for Item_ID=%u: offset is %lld (pos=%lld,len=%lld) while file size is %llu",
                         itemId, span.first + span.second, span.first, span.second, filesize);
          }
        }
      }
    },
    {
      "Section 8.11.12.1\n"
      "Zero or one 'iref' box per MetaBox",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
          {
            auto const a = arity(box, FOURCC("iref"));

            if(a != 0 && a != 1)
              out->error("There shall be zero or one 'iref' box per MetaBox, found %d", a);
          }
      }
    },
    {
      "Section 8.11.14.1\n"
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
              out->error("There shall be at most one occurrence of a given item_ID but item_ID=%u found several times", itemId);
      }
    },
    {
      "Section 8.11.14.1\n"
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
    },
    {
      "Section 8.11.14.1\n"
      "ItemPropertyContainerBox: flags should be equal to 0 unless there are more than\n"
      "127 properties in the ItemPropertyContainerBox",
      [] (Box const& root, IReport* out)
      {
        int64_t ipmaFlags = 0;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                    for(auto& sym : iprpChild.syms)
                      if(!strcmp(sym.name, "flags"))
                        ipmaFlags = sym.value;

        if(ipmaFlags > 0)
          for(auto& box : root.children)
            if(box.fourcc == FOURCC("meta"))
              for(auto& metaChild : box.children)
                if(metaChild.fourcc == FOURCC("iprp"))
                  for(auto& iprpChild : metaChild.children)
                    if(iprpChild.fourcc == FOURCC("ipco"))
                      if(iprpChild.children.size() <= 127)
                        out->warning("'ipma' flags should be equal to 0 unless there are more than 127 properties in"
                                     " the ItemPropertyContainerBox (found %d)", (int)iprpChild.children.size());
      }
    },
    {
      "Section 8.11.14.1\n"
      "ItemPropertyContainerBox: version 0 should be used unless 32-bit item_ID values\n"
      "are needed",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iprp"))
                for(auto& iprpChild : metaChild.children)
                  if(iprpChild.fourcc == FOURCC("ipma"))
                  {
                    int64_t ipmaVersion = -1;
                    std::vector<uint32_t> itemIds;

                    for(auto& sym : iprpChild.syms)
                    {
                      if(!strcmp(sym.name, "version"))
                        ipmaVersion = sym.value;

                      if(!strcmp(sym.name, "item_ID"))
                        itemIds.push_back(sym.value);
                    }

                    bool itemIdsRequire32bits = false;

                    for(auto i : itemIds)
                      if(i & 0xFF00)
                        itemIdsRequire32bits = true;

                    if(ipmaVersion > 0 && !itemIdsRequire32bits)
                    {
                      std::string itemIdsStr;

                      for(auto i : itemIds)
                        itemIdsStr += std::to_string(i) + " ";

                      out->warning("'ipma' version 0 should be used unless 32-bit item_ID values are needed (Item_IDs = { %s})", itemIdsStr.c_str());
                    }
                  }
      }
    },
    {
      "Section 4.2\n"
      "if size is 0, then this box shall be in a top-level container, and be the last box in that container",
      [] (Box const& root, IReport* out)
      {
        if(root.children.empty())
          return;

        std::vector<const Box*> zeroSizedBoxes;
        findZeroSizedBoxes(&root, zeroSizedBoxes);

        for(auto box : zeroSizedBoxes)
          if(box != &root.children[root.children.size() - 1])
            out->error("Box '%s' has size 0 but is not last int the top-level container", toString(box->fourcc).c_str());
      }
    },
    {
      "Section 6.1.2\n"
      "The sequence of objects in the file shall contain exactly one presentation\n"
      "metadata wrapper (the MovieBox).",
      [] (Box const& root, IReport* out)
      {
        int numMoov = 0;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("moov"))
            numMoov++;

        if(numMoov > 1)
          out->error("There shall be one 'moov' at most, found %d", numMoov);
      }
    },
    {
      "Table 1: box structure and arity\n"
      "This is rather a safety check than a formal rule",
      [] (Box const& root, IReport* out)
      {
        typedef std::pair<unsigned, unsigned> Span;

        auto boxCheck = [&] (std::vector<uint32_t> oneOf4CCs, uint32_t parent4CC, Span expectedArity) {
            auto parents = findBoxes(root, parent4CC);

            for(auto& parent : parents)
            {
              unsigned arityFromExpectedParent = 0, arityFromBoxes = 0;

              for(auto fourcc : oneOf4CCs)
              {
                unsigned localArity = 0;

                if(parent->fourcc == fourcc)
                  localArity++;

                for(auto& child : parent->children)
                  if(child.fourcc == fourcc)
                    localArity++;

                auto boxes = findBoxes(*parent, fourcc);
                arityFromBoxes += boxes.size();
                arityFromExpectedParent += localArity;
              }

              {
                std::string str;

                for(auto fourcc : oneOf4CCs)
                  str += toString(fourcc) + " ";

                if(arityFromBoxes != arityFromExpectedParent)
                  out->error("Found %u boxes { %s} but expected %u with parent '%s'. Check your box structure.",
                             arityFromBoxes, str.c_str(), arityFromExpectedParent, toString(parent4CC).c_str());

                if(arityFromExpectedParent < expectedArity.first || arityFromExpectedParent > expectedArity.second)
                  out->error("Wrong arity for boxes { %s} in parent '%s': expected in range [%u-%u], found %u",
                             str.c_str(), toString(parent4CC).c_str(), expectedArity.first, expectedArity.second, arityFromExpectedParent);
              }
            }
          };

        boxCheck({ FOURCC("ftyp") }, FOURCC("root"), { 1, 1 });
        boxCheck({ FOURCC("pdin") }, FOURCC("root"), { 0, 1 });

        boxCheck({ FOURCC("moov") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("mvhd") }, FOURCC("moov"), { 0, 1 });
        boxCheck({ FOURCC("meta") }, FOURCC("moov"), { 0, 1 });
        boxCheck({ FOURCC("trak") }, FOURCC("moov"), { 1, INT32_MAX });
        boxCheck({ FOURCC("tkhd") }, FOURCC("trak"), { 1, 1 });
        boxCheck({ FOURCC("tref") }, FOURCC("trak"), { 0, 1 });
        boxCheck({ FOURCC("trgr") }, FOURCC("trak"), { 0, 1 });
        boxCheck({ FOURCC("edts") }, FOURCC("trak"), { 0, 1 });
        boxCheck({ FOURCC("elst") }, FOURCC("edts"), { 0, 1 });
        boxCheck({ FOURCC("meta") }, FOURCC("trak"), { 0, 1 });
        boxCheck({ FOURCC("mdia") }, FOURCC("trak"), { 1, 1 });
        boxCheck({ FOURCC("mdhd") }, FOURCC("mdia"), { 1, 1 });
        boxCheck({ FOURCC("hdlr") }, FOURCC("mdia"), { 1, 1 });
        boxCheck({ FOURCC("elng") }, FOURCC("mdia"), { 0, 1 });
        boxCheck({ FOURCC("minf") }, FOURCC("mdia"), { 1, 1 });
        boxCheck({ FOURCC("vmhd"), FOURCC("smhd"), FOURCC("hmhd"), FOURCC("sthd"), FOURCC("nmhd") }, FOURCC("minf"), { 1, 1 });
        boxCheck({ FOURCC("dinf") }, FOURCC("minf"), { 1, 1 });
        boxCheck({ FOURCC("dref") }, FOURCC("dinf"), { 1, INT32_MAX });
        boxCheck({ FOURCC("stbl") }, FOURCC("minf"), { 1, 1 });
        boxCheck({ FOURCC("stsd") }, FOURCC("stbl"), { 1, 1 });
        boxCheck({ FOURCC("stts") }, FOURCC("stbl"), { 1, 1 });
        boxCheck({ FOURCC("ctts") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("cslg") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("stsc") }, FOURCC("stbl"), { 1, 1 });
        boxCheck({ FOURCC("stsz"), FOURCC("stz2") }, FOURCC("stbl"), { 1, 1 });
        boxCheck({ FOURCC("stco"), FOURCC("co64") }, FOURCC("stbl"), { 1, 1 });
        boxCheck({ FOURCC("stss") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("stsh") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("padb") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("stdp") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("sdtp") }, FOURCC("stbl"), { 0, 1 });
        boxCheck({ FOURCC("sbgp") }, FOURCC("stbl"), { 0, INT32_MAX });
        boxCheck({ FOURCC("sgpd") }, FOURCC("stbl"), { 0, INT32_MAX }); // Zero or more, with exactly one for each grouping_type in a SampleToGroupBox
        boxCheck({ FOURCC("subs") }, FOURCC("stbl"), { 0, INT32_MAX });
        boxCheck({ FOURCC("saiz") }, FOURCC("stbl"), { 0, INT32_MAX });
        boxCheck({ FOURCC("saio") }, FOURCC("stbl"), { 0, INT32_MAX });
        boxCheck({ FOURCC("udta") }, FOURCC("trak"), { 0, 1 });
        boxCheck({ FOURCC("cprt") }, FOURCC("udta"), { 0, INT32_MAX });
        boxCheck({ FOURCC("tsel") }, FOURCC("udta"), { 0, 1 });
        boxCheck({ FOURCC("kind") }, FOURCC("udta"), { 0, INT32_MAX });
        boxCheck({ FOURCC("strk") }, FOURCC("udta"), { 0, INT32_MAX });
        boxCheck({ FOURCC("stri") }, FOURCC("strk"), { 1, 1 });
        boxCheck({ FOURCC("strd") }, FOURCC("strk"), { 1, 1 });
        boxCheck({ FOURCC("ludt") }, FOURCC("udta"), { 0, INT32_MAX });
        boxCheck({ FOURCC("mvex") }, FOURCC("moov"), { 0, INT32_MAX });
        boxCheck({ FOURCC("mehd") }, FOURCC("mvex"), { 0, INT32_MAX });
        boxCheck({ FOURCC("trex") }, FOURCC("mvex"), { 0, INT32_MAX });
        boxCheck({ FOURCC("leva") }, FOURCC("mvex"), { 1, 1 });
        boxCheck({ FOURCC("udta") }, FOURCC("moov"), { 0, 1 });
        boxCheck({ FOURCC("moov") }, FOURCC("udta"), { 0, INT32_MAX });

        boxCheck({ FOURCC("moof") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("mfhd") }, FOURCC("moof"), { 1, 1 });
        boxCheck({ FOURCC("meta") }, FOURCC("moof"), { 0, 1 });
        boxCheck({ FOURCC("traf") }, FOURCC("moof"), { 0, INT32_MAX });
        boxCheck({ FOURCC("tfhd") }, FOURCC("traf"), { 1, 1 });
        boxCheck({ FOURCC("trun") }, FOURCC("traf"), { 0, INT32_MAX });
        boxCheck({ FOURCC("sbgp") }, FOURCC("traf"), { 0, INT32_MAX });
        boxCheck({ FOURCC("sgpd") }, FOURCC("traf"), { 0, INT32_MAX });
        boxCheck({ FOURCC("subs") }, FOURCC("traf"), { 0, INT32_MAX });
        boxCheck({ FOURCC("saiz") }, FOURCC("traf"), { 0, INT32_MAX });
        boxCheck({ FOURCC("saio") }, FOURCC("traf"), { 0, INT32_MAX });
        boxCheck({ FOURCC("tfdt") }, FOURCC("traf"), { 0, 1 });
        boxCheck({ FOURCC("meta") }, FOURCC("traf"), { 0, 1 });
        boxCheck({ FOURCC("udta") }, FOURCC("traf"), { 0, 1 });
        boxCheck({ FOURCC("udta") }, FOURCC("moof"), { 0, 1 });

        boxCheck({ FOURCC("mfra") }, FOURCC("root"), { 0, 1 });
        boxCheck({ FOURCC("tfra") }, FOURCC("mfra"), { 0, 1 }); // TODO: one per track
        boxCheck({ FOURCC("mfro") }, FOURCC("mfra"), { 1, 1 });

        boxCheck({ FOURCC("mdat") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("free") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("skip") }, FOURCC("root"), { 0, INT32_MAX });

        boxCheck({ FOURCC("meta") }, FOURCC("root"), { 0, 1 });
        boxCheck({ FOURCC("hdlr") }, FOURCC("meta"), { 1, 1 });
        boxCheck({ FOURCC("dinf") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("dref") }, FOURCC("dinf"), { 1, 1 });
        // boxCheck({ FOURCC("url "), FOURCC("urn ") }, FOURCC("dinf"), { 1, INT32_MAX }); //TODO: cannot be expressed for now
        boxCheck({ FOURCC("iloc") }, FOURCC("meta"), { 0, INT32_MAX });
        boxCheck({ FOURCC("ipro") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("sinf") }, FOURCC("ipro"), { 1, INT32_MAX });
        boxCheck({ FOURCC("frma") }, FOURCC("sinf"), { 1, 1 });
        boxCheck({ FOURCC("schm") }, FOURCC("frma"), { 0, 1 }); // Zero or one in 'sinf', depending on the protection structure; Exactly one in 'rinf' and 'srpp'
        boxCheck({ FOURCC("schi") }, FOURCC("frma"), { 0, 1 });
        boxCheck({ FOURCC("iinf") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("xml ") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("bxml") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("pitm") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("fiin") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("paen") }, FOURCC("fiin"), { 0, 1 });
        boxCheck({ FOURCC("fire") }, FOURCC("paen"), { 0, 1 });
        boxCheck({ FOURCC("fpar") }, FOURCC("paen"), { 1, 1 });
        boxCheck({ FOURCC("fecr") }, FOURCC("paen"), { 0, 1 });
        boxCheck({ FOURCC("segr") }, FOURCC("fiin"), { 0, 1 });
        boxCheck({ FOURCC("gitn") }, FOURCC("fiin"), { 0, 1 });
        boxCheck({ FOURCC("idat") }, FOURCC("meta"), { 0, 1 });
        boxCheck({ FOURCC("iref") }, FOURCC("meta"), { 0, 1 });

        boxCheck({ FOURCC("meco") }, FOURCC("root"), { 0, 1 });
        boxCheck({ FOURCC("mere") }, FOURCC("meco"), { 0, INT32_MAX });
        boxCheck({ FOURCC("meta") }, FOURCC("mere"), { 0, 1 });

        boxCheck({ FOURCC("styp") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("sidx") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("ssix") }, FOURCC("root"), { 0, INT32_MAX });
        boxCheck({ FOURCC("prft") }, FOURCC("root"), { 0, INT32_MAX });
      }
    },
  },
  nullptr,
};
}

static auto const registered = registerSpec(&specIsobmff);

