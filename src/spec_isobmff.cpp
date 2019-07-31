#include "spec.h"
#include <cstring>
#include <functional>
#include <vector>
#include <queue>

static const SpecDesc spec =
{
  "isobmff",
  "ISO Base Media File Format\n"
  "MPEG-4 part 22 - ISO/IEC 14496-12 - MPEG-4 Part 12 - m17277 (6th+FDAM1+FDAM2+COR1-R4)",
  {
    {
      "Section: 12.1.3.2\n"
      "CleanApertureBox 'clap' and PixelAspectRatioBox 'pasp' in VisualSampleEntry",
      [] (Box const& root, IReport* out)
      {
        std::vector<uint32_t> FourCCs;
        std::vector<const Box*> found;

        auto checkIntegrityPasp = [&] (const Box& pasp) {
            if(pasp.size != 12)
              out->error("'pasp' box size is %d bytes (expected 12).", pasp.size);

            for(auto& field : pasp.syms)
              if(strcmp(field.name, "max_content_light_level") && strcmp(field.name, "max_pic_average_light_level"))
                out->error("Invalid 'pasp' field \"%s\" (value=%lld).", field.name, field.value);
          };

        auto checkIntegrityClap = [&] (const Box& clap) {
            if(clap.size != 36)
              out->error("'clap' box size is %d bytes (expected 36).", clap.size);
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
                                  if(stsdChild.fourcc == FOURCC("avc1"))
                                    for(auto& avc1Child : stsdChild.children)
                                      if(avc1Child.fourcc == FOURCC("clap") || avc1Child.fourcc == FOURCC("pasp"))
                                      {
                                        FourCCs.push_back(avc1Child.fourcc);
                                        found.push_back(&avc1Child);
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
                  checkIntegrity(box);
                  out->error("Invalid '%s' position (parent is '%s').", toString(fourCC).c_str(), toString(parent.fourcc).c_str());
                }
              }
              else
                parse(box, fourCC, checkIntegrity);
          };

        parse(root, FOURCC("clap"), checkIntegrityClap);
        parse(root, FOURCC("pasp"), checkIntegrityPasp);
      },
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

