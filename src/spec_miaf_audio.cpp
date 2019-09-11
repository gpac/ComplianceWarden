#include "spec.h"
#include "fourcc.h"
#include <cstring>

extern bool isMpegAudio(uint8_t oti);

// MPEG-4 AAC-LC, HE-AAC Level 2, or HE-AACv2 Level 2
static bool isMiafAac(const Box& mp4a, IReport* out)
{
  bool res = false;

  for(auto& child : mp4a.children)
    if(child.fourcc == FOURCC("esds"))
    {
      res = true;

      for(auto sym : child.syms)
      {
        if(!strcmp(sym.name, "streamType"))
          if(sym.value != 5)
          {
            out->error("streamType shall be 5 (found %lld)", sym.value);
            res = false;
          }

        if(!strcmp(sym.name, "objectTypeIndication"))
          if(!isMpegAudio(sym.value))
          {
            out->error("objectTypeIndication shall be an AAC codec (found %lld)", sym.value);
            res = false;
          }

        if(!strcmp(sym.name, "audioObjectType"))
          switch(sym.value)
          {
          case 2: case 5: case 29: break;
          default:
            out->error("audioObjectType shall be 2, 5 or 29 (found %lld)", sym.value);
            res = false;
          }
      }
    }

  return res;
}

const std::initializer_list<RuleDesc> rulesAudio =
{
  {
    "Section 7.4.5\n"
    "Each stream shall be coded as either a variant of AAC as defined in\n"
    "ISO/IEC 14496-3, or as uncompressed two’s-complement",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("mdia"))
                {
                  bool audio = false;

                  for(auto& mdiaChild : trakChild.children)
                    if(mdiaChild.fourcc == FOURCC("hdlr"))
                      for(auto& sym : mdiaChild.syms)
                        if(!strcmp(sym.name, "handler_type"))
                          if(sym.value == FOURCC("soun"))
                            audio = true;

                  if(audio)
                  {
                    for(auto& mdiaChild : trakChild.children)
                      if(mdiaChild.fourcc == FOURCC("minf"))
                        for(auto& minfChild : mdiaChild.children)
                          if(minfChild.fourcc == FOURCC("stbl"))
                            for(auto& stblChild : minfChild.children)
                              if(stblChild.fourcc == FOURCC("stsd"))
                                for(auto& stsdChild : stblChild.children)
                                {
                                  if(stsdChild.fourcc != FOURCC("twos") && !isMiafAac(stsdChild, out))
                                    out->error("Audio shall be a variant of AAC as defined in Section 7.4.5 or as uncompressed two’s-complement");
                                }
                  }
                }
    }
  },
  {
    "Section 7.4.5\n"
    "If uncompressed audio is chosen, it shall be mono or stereo audio",
    [] (Box const& root, IReport* out)
    {
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
                                if(stsdChild.fourcc == FOURCC("twos"))
                                  for(auto& sym : stsdChild.syms)
                                    if(!strcmp(sym.name, "channelcount"))
                                      if(sym.value > 2)
                                        out->error("Uncompressed audio of type 'twos' found shall be mono or stereo, found %d channels", sym.value);
    }
  },
  {
    "Section 7.4.5\n"
    "If AAC is chosen, then each AAC elementary stream shall be encoded using\n"
    "MPEG-4 AAC-LC, HE-AAC Level 2, or HE-AACv2 Level 2.",
    [] (Box const& root, IReport* out)
    {
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
                                if(stsdChild.fourcc == FOURCC("mp4a"))
                                  if(!isMiafAac(stsdChild, out))
                                    out->error("Audio shall be MPEG-4 AAC-LC, HE-AAC Level 2, or HE-AACv2 Level 2");
    }
  },
  {
    "Section 7.4.5\n"
    "If AAC is chosen, then when using HE-AAC and HE-AACv2 bitstreams, explicit\n"
    "backwards compatible signalling shall be used to indicate the use of the\n"
    "spectral bandwidth replication (SBR) and parametric stereo (PS) coding tools.",
    [] (Box const& root, IReport* out)
    {
      (void)root;
      (void)out;
      // TODO
    }
  },
  {
    "Section 7.4.5\n"
    "If AAC is chosen, then AAC shall not exceed two audio channels",
    [] (Box const& root, IReport* out)
    {
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
                                if(stsdChild.fourcc == FOURCC("mp4a"))
                                  for(auto& sym : stsdChild.syms)
                                    if(!strcmp(sym.name, "channelcount"))
                                      if(sym.value > 2)
                                        out->error("channelcount shall not exceed 2, found %lld", sym.value);
    }
  },
  {
    "Section 7.4.5\n"
    "If AAC is chosen, then AAC elementary streams shall not exceed 48 kHz sampling rate",
    [] (Box const& root, IReport* out)
    {
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
                                if(stsdChild.fourcc == FOURCC("mp4a"))
                                  for(auto& sym : stsdChild.syms)
                                    if(!strcmp(sym.name, "samplerate"))
                                      if((sym.value >> 16) > 48000)
                                        out->error("Sampling rate shall not exceed 48000, found %lld", sym.value >> 16);
    }
  },
};
