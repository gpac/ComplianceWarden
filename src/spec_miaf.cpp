#include "spec.h"
#include "fourcc.h"
#include <cassert>
#include <cstring>
#include <functional>
#include <map>

extern bool isVisualSampleEntry(uint32_t fourcc);

static bool boxEqual(Box const* a, Box const* b)
{
  if(a->fourcc != b->fourcc)
    return false;

  if(a->size != b->size)
    return false;

  if(a->syms.size() != b->syms.size())
    return false;

  for(size_t i = 0; i < a->syms.size(); ++i)
  {
    if(strcmp(a->syms[i].name, b->syms[i].name))
      return false;

    if(a->syms[i].value != b->syms[i].value)
      return false;
  }

  if(a->children.size() != b->children.size())
    return false;

  for(size_t i = 0; i < a->children.size(); ++i)
    if(!boxEqual(&a->children[i], &b->children[i]))
      return false;

  return true;
}

std::initializer_list<RuleDesc> rulesGeneral =
{
  {
    "Section 7.2.1.4:\n"
    "The file-level MetaBox shall always be present.\n"
    "The MetaBox shall be present at the file-level",
    [] (Box const& root, IReport* out)
    {
      bool found = false;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          found = true;

      if(!found)
        out->error("'meta' box not found at file level");
    }
  },
  {
    "Section 7.2.1.1\n"
    "The HandlerBox shall be the first contained box within the MetaBox.",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
        {
          if(box.children.empty() || box.children[0].fourcc != FOURCC("hdlr"))
            out->error("The HandlerBox shall be the first contained box within the MetaBox");
        }
    }
  },
  {
    "Section 7.2.1.2\n"
    "The FileTypeBox shall contain, in the compatible_brands list,\n"
    "the following (in any order): 'mif1' (specified in ISO/IEC 23008-12)\n"
    "[and] brand(s) identifying conformance to this document (specified in 10).\n"
    "[...]\n"
    "Files conforming to the general restrictions in clause 7 shall include\n"
    "the brand 'miaf' in the compatible_brands in the FileTypeBox.",
    [] (Box const& root, IReport* out)
    {
      if(root.children.empty() || root.children[0].fourcc != FOURCC("ftyp"))
      {
        out->error("'ftyp' box not found");
        return;
      }

      auto& ftypBox = root.children[0];

      bool foundMiaf = false, foundMif1 = false;

      for(auto& brand : ftypBox.syms)
      {
        if(strcmp(brand.name, "compatible_brand"))
          continue;

        if(brand.value == FOURCC("miaf"))
          foundMiaf = true;

        if(brand.value == FOURCC("mif1"))
          foundMif1 = true;
      }

      auto strFound = [] (bool found) {
          return found ? "found" : "not found";
        };

      if(!foundMiaf || !foundMif1)
        out->error("compatible_brands list shall contain 'miaf' (%s) and 'mif1' (%s)", strFound(foundMiaf), strFound(foundMif1));
    },
  },
  {
    "Section 7.2.1.4\n"
    "The XMLBox and BinaryXMLBox shall not be used in a MetaBox.",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
      {
        if(box.fourcc == FOURCC("meta"))
        {
          for(auto& metaChild : box.children)
          {
            if(metaChild.fourcc == FOURCC("xml "))
              out->error("MetaBox shall not contain a XMLBox");

            if(metaChild.fourcc == FOURCC("bxml"))
              out->error("MetaBox shall not contain a BinaryXMLBox");
          }
        }
      }
    },
  },
  {
    "Section 7.2.1.5\n"
    "The handler type for the MetaBox shall be 'pict'.",
    [] (Box const& root, IReport* out)
    {
      bool found = false;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("hdlr"))
              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "handler_type"))
                {
                  found = true;

                  if(field.value != FOURCC("pict"))
                    out->error("The handler type for the MetaBox shall be 'pict'");
                }

      if(!found)
        out->error("'hdlr' not found in MetaBox");
    },
  },
  {
    "Section 7.2.1.7\n"
    "construction_method shall be equal to 0 for MIAF image items that are coded image items.\n"
    "construction_method shall be equal to 0 or 1 for MIAF image items that are derived image items.",
    [] (Box const& root, IReport* out)
    {
      struct ImageItem
      {
        bool foundIref = false;
        int constructionMethod = -1;
      };
      std::map<uint32_t /*itemId*/, ImageItem> imageItems;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
        {
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iloc"))
            {
              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "item_ID"))
                {
                  if(imageItems.find(field.value) != imageItems.end())
                    out->error("iloc itemId duplicates for %lld", field.value);

                  ImageItem ii;
                  imageItems.insert({ field.value, ii });
                }

              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "construction_method"))
                  imageItems[field.value].constructionMethod = field.value;
            }

          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iref"))
              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "to_item_ID"))
                {
                  if(imageItems.find(field.value) == imageItems.end())
                    out->error("iref references a non existing itemId=%lld", field.value);
                  else
                    imageItems[field.value].foundIref = true;
                }
        }

      for(auto& ii : imageItems)
        if(ii.second.constructionMethod == 1 && !ii.second.foundIref)
          out->error("construction_method=1 on a coded image item");
    }
  },
  {
    "Section 7.2.1.8\n"
    "MIAF image items shall not reference any item protection",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
        {
          bool isMiafItem = false;

          for(auto& metaChild : box.children)
          {
            if(metaChild.fourcc == FOURCC("hdlr"))
            {
              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "handler_type"))
                {
                  if(field.value == FOURCC("pict"))
                    isMiafItem = true;
                }
            }
            else if(metaChild.fourcc == FOURCC("ipro"))
            {
              if(isMiafItem)
                out->error("MIAF image item shall not reference any item protection ('ipro')");
            }
            else if(metaChild.fourcc == FOURCC("iinf"))
              for(auto& iinfChild : metaChild.children)
                if(iinfChild.fourcc == FOURCC("infe"))
                  for(auto& sym : iinfChild.syms)
                    if(!strcmp(sym.name, "item_protection_index"))
                      if(sym.value != 0)
                        out->error("MIAF image item shall not reference any item protection ('infe' item_protection_index)");
          }
        }
    },
  },
  {
    "Section 7.3.2\n"
    "The primary item shall be a MIAF master image item.",
    [] (Box const& root, IReport* out)
    {
      bool found = false;
      uint32_t primaryItemId = -1;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("pitm"))
            {
              found = true;

              for(auto& field : metaChild.syms)
                if(!strcmp(field.name, "item_ID"))
                  primaryItemId = field.value;
            }

      if(!found)
        out->error("PrimaryItemBox is required");

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iref"))
            {
              uint32_t fourcc = 0;

              for(auto& field : metaChild.syms)
              {
                if(!strcmp(field.name, "box_type"))
                  fourcc = field.value;

                if(!strcmp(field.name, "from_item_ID"))
                  if(field.value == primaryItemId)
                  {
                    if(fourcc == FOURCC("auxl"))
                      out->error("MIAF master image (item_ID=%X) shall not be a auxiliary image", primaryItemId);

                    if(fourcc == FOURCC("thmb"))
                      out->error("MIAF master image (item_ID=%X) shall not be a thumbnail image", primaryItemId);
                  }
              }
            }
    },
  },
  {
    "Section 7.3.6.3\n"
    "Every image item shall be associated with a Image spatial extents property",
    [] (Box const& root, IReport* out)
    {
      bool found = false;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
              {
                if(iprpChild.fourcc == FOURCC("ipco"))
                  for(auto& ipcoChild : iprpChild.children)
                    if(ipcoChild.fourcc == FOURCC("ispe"))
                      found = true;

                break;
              }

      if(!found)
        out->error("MIAF missing Image spatial extents property");
    },
  },
  {
    "Section 7.4.4.2.2\n"
    "Enforce 'clli' parsing and positioning",
    [] (Box const& root, IReport* out)
    {
      std::vector<const Box*> found;

      auto checkIntegrity = [&] (const Box& clli) {
          if(clli.size != 12)
            out->error("'clli' box size is %d bytes (expected 12)", clli.size);

          for(auto& field : clli.syms)
            if(strcmp(field.name, "max_content_light_level") && strcmp(field.name, "max_pic_average_light_level"))
              out->error("Invalid 'clli' field \"%s\" (value=%lld)", field.name, field.value);
        };

      // Look for valid 'clli' boxes
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
                                    if(sampleEntryChild.fourcc == FOURCC("clli"))
                                    {
                                      auto& clli = sampleEntryChild;
                                      found.push_back(&clli);
                                      checkIntegrity(clli);
                                    }

      // Look for other invalidly positioned 'clli' boxes
      std::function<void(const Box &)> parse = [&] (const Box& parent)
        {
          for(auto& box : parent.children)
            if(box.fourcc == FOURCC("clli"))
            {
              bool ok = false;

              for(auto b : found)
                if(&box == b)
                  ok = true;

              if(!ok)
              {
                checkIntegrity(box);
                out->error("Invalid 'clli' position (parent is '%s')", toString(parent.fourcc).c_str());
              }
            }
            else
              parse(box);
        };

      parse(root);
    },
  },
  {
    "Section 7.4.4.3.2\n"
    "Enforce 'mdcv' parsing and positioning",
    [] (Box const& root, IReport* out)
    {
      std::vector<const Box*> found;

      auto checkIntegrity = [&] (const Box& mdcv) {
          if(mdcv.size != 32)
            out->error("'mdcv' box size is %d bytes (expected 32)", mdcv.size);

          for(auto& field : mdcv.syms)
            if(strcmp(field.name, "display_primaries_x_0") && strcmp(field.name, "display_primaries_y_0")
               && strcmp(field.name, "display_primaries_x_1") && strcmp(field.name, "display_primaries_y_1")
               && strcmp(field.name, "display_primaries_x_2") && strcmp(field.name, "display_primaries_y_2")
               && strcmp(field.name, "white_point_x") && strcmp(field.name, "white_point_y")
               && strcmp(field.name, "max_display_mastering_luminance") && strcmp(field.name, "min_display_mastering_luminance"))
              out->error("Invalid 'mdcv' field \"%s\" (value=%lld)", field.name, field.value);
        };

      // Look for valid 'mdcv' boxes
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
                                    if(sampleEntryChild.fourcc == FOURCC("mdcv"))
                                    {
                                      auto& mdcv = sampleEntryChild;
                                      found.push_back(&mdcv);
                                      checkIntegrity(mdcv);
                                    }

      // Look for other invalidly positioned 'mdcv' boxes
      std::function<void(const Box &)> parse = [&] (const Box& parent)
        {
          for(auto& box : parent.children)
            if(box.fourcc == FOURCC("mdcv"))
            {
              bool ok = false;

              for(auto b : found)
                if(&box == b)
                  ok = true;

              if(!ok)
              {
                checkIntegrity(box);
                out->error("Invalid 'mdcv' position (parent is '%s')", toString(parent.fourcc).c_str());
              }
            }
            else
              parse(box);
        };

      parse(root);
    },
  },
  {
    "Section 7.3.6.5\n"
    "\"all images conformant with this document shall have a pixel aspect ratio of 1:1\"\n"
    "The default pixel aspect ratio of an image, in the absence of association with\n"
    "a PixelAspectRatioBox property, is 1:1 (i.e. 'square'). This value of pixel\n"
    "aspect ratio is the mandatory value for all images mandated or conditionally\n"
    "mandated by this document, i.e. all images conformant with this document shall\n"
    "have a pixel aspect ratio of 1:1.",
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
                                if(isVisualSampleEntry(stsdChild.fourcc))
                                  for(auto& sampleEntryChild : stsdChild.children)
                                    if(sampleEntryChild.fourcc == FOURCC("pasp"))
                                    {
                                      uint32_t hSpacing = -1, vSpacing = -1;

                                      for(auto& sym : sampleEntryChild.syms)
                                      {
                                        if(!strcmp(sym.name, "hSpacing"))
                                          hSpacing = sym.value;
                                        else if(!strcmp(sym.name, "vSpacing"))
                                          vSpacing = sym.value;
                                      }

                                      if(vSpacing != hSpacing)
                                        out->error("Pixel aspect ratio shall be 1:1 but is %:%", hSpacing, vSpacing);
                                    }
    },
  },
  {
    "Section 7.4.3\n"
    "The use of the matrix field of the TrackHeaderBox shall represent only no\n"
    "transformation (identity matrix), or vertical or horizontal mirroring and/or\n"
    "rotations of 90°, 180° or 270°.",
    [] (Box const& root, IReport* out)
    {
      std::vector<int64_t> matrix; // a, b, u, c, d, v, x, y, w

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("tkhd"))
                  for(auto& sym : trakChild.syms)
                    if(!strcmp(sym.name, "matrix"))
                      matrix.push_back(sym.value);

      if(!matrix.empty())
      {
        // either a or c but not both shall be equal to 0x00010000 or 0xFFFF0000, while the remaining one of a and c shall be equal to 0;
        if(matrix[0])
        {
          if(matrix[0] != 0x00010000 && matrix[0] != 0xffff0000)
            out->error("Matrix field of the TrackHeaderBox: \"a\" value (%llX) shall be 0x00010000 or 0xFFFF0000", matrix[0]);

          if(matrix[3])
            out->error("Matrix field of the TrackHeaderBox: \"a\" (%llX) or \"c\" (%llX) shall be equal to 0", matrix[0], matrix[3]);
        }
        else if(matrix[3] != 0x00010000 && matrix[3] != 0xffff0000)
          out->error("Matrix field of the TrackHeaderBox: \"c\" value (%llX) shall be 0x00010000 or 0xFFFF0000 (\"a\" is 0)", matrix[3]);

        // either b or d but not both shall be equal to 0x00010000 or 0xFFFF0000, while the remaining one of b and d shall be equal to 0;
        if(matrix[1])
        {
          if(matrix[1] != 0x00010000 && matrix[1] != 0xffff0000)
            out->error("Matrix field of the TrackHeaderBox: \"b\" value (%llX) shall be 0x00010000 or 0xFFFF0000", matrix[1]);

          if(matrix[4])
            out->error("Matrix field of the TrackHeaderBox: \"b\" (%llX) or \"d\" (%llX) shall be equal to 0", matrix[1], matrix[4]);
        }
        else if(matrix[4] != 0x00010000 && matrix[4] != 0xffff0000)
          out->error("Matrix field of the TrackHeaderBox: \"d\" value (%llX) shall be 0x00010000 or 0xFFFF0000 (\"b\" is 0)", matrix[5]);

        if(matrix[2])
          out->error("Matrix field of the TrackHeaderBox: \"u\" value (%llX) shall be 0", matrix[2]);

        if(matrix[5])
          out->error("Matrix field of the TrackHeaderBox: \"v\" value (%llX) shall be 0", matrix[5]);

        if(matrix[8] != 0x40000000)
          out->error("Matrix field of the TrackHeaderBox: \"w\" value (%llX) shall be 0x40000000", matrix[8]);
      }
    },
  },
  {
    "Section 8.7\n"
    "If multiple tracks are present in the file, they shall have the same duration,\n"
    "and edit lists may be needed to achieve this.",
    [] (Box const& root, IReport* out)
    {
      std::vector<int64_t> durations;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("tkhd"))
                  for(auto& sym : trakChild.syms)
                    if(!strcmp(sym.name, "duration"))
                      durations.push_back(sym.value);

      if(!durations.empty())
      {
        auto refDuration = durations[0];

        for(auto d : durations)
          if(d != refDuration)
            out->error("All tracks shall have the same duration: found (%lld) while first track duration is %lld (expressed in 'mvhd' timescale)", d, refDuration);
      }
    },
  },
  {
    "Section 7.4.5\n"
    "When present, audio tracks shall have the same duration as the video or image\n"
    "sequence track",
    [] (Box const& root, IReport* out)
    {
      std::vector<int64_t> durations;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("tkhd"))
                  for(auto& sym : trakChild.syms)
                    if(!strcmp(sym.name, "duration"))
                      durations.push_back(sym.value);

      if(!durations.empty())
      {
        auto refDuration = durations[0];

        for(auto d : durations)
          if(d != refDuration)
            out->error("All tracks shall have the same duration: found (%lld) while first track duration is %lld (expressed in 'mvhd' timescale)", d, refDuration);
      }
    },
  },
  {
    "Section 8.5\n"
    "For any type of track (video, audio, image sequence, metadata etc.) there shall either\n"
    "be exactly one track of that type not identified as an auxiliary or thumbnail track,\n"
    "or all tracks of that type not identified as an auxiliary or thumbnail track shall form a\n"
    "single group of alternates, i.e. after any alternate track selection has been performed,\n"
    "there is at most a single track of any given type",
    [] (Box const& root, IReport* out)
    {
      struct HandlerTypeData
      {
        uint16_t alternateGroup = 0;
        std::vector<uint32_t> referenceTypes;
      };
      std::map<uint32_t /*handler_type*/, HandlerTypeData> trackTypes;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
            {
              uint32_t handlerType = 0;

              // find the hldr
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("mdia"))
                  for(auto& mdiaChild : trakChild.children)
                    if(mdiaChild.fourcc == FOURCC("hdlr"))
                      for(auto& sym : mdiaChild.syms)
                        if(!strcmp(sym.name, "handler_type"))
                          handlerType = (uint32_t)sym.value;

              if(handlerType == 0)
              {
                out->error("'trak' with no 'hdlr': check ISOBMFF conformance");
                continue;
              }

              // ensure entry
              if(trackTypes.find(handlerType) == trackTypes.end())
                trackTypes.insert({ handlerType, HandlerTypeData {}
                                  });

              // find alternate_group
              {
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tkhd"))
                    for(auto& sym : trakChild.syms)
                      if(!strcmp(sym.name, "alternate_group"))
                        trackTypes.find(handlerType)->second.alternateGroup = (uint16_t)sym.value;
              }

              // find tref
              bool trefFound = false;
              {
                for(auto& trakChild : moovChild.children)
                  if(trakChild.fourcc == FOURCC("tref"))
                    for(auto& trefChild : trakChild.children)
                    {
                      if(trefFound)
                        out->error("'tref' with multiple TrackReferenceTypeBox: check ISOBMFF conformance");

                      if(trefChild.fourcc != FOURCC("thmb") && trefChild.fourcc != FOURCC("auxl"))
                        out->error("'tref' with unknown TrackReferenceTypeBox type \'%X\'", trefChild.fourcc);
                      else if(handlerType != FOURCC("pict"))
                        out->error("Thumbnails and Auxiliaries are expected with a 'pict' handler type, found '%s'", toString(handlerType).c_str());

                      trefFound = true;
                      trackTypes.find(handlerType)->second.referenceTypes.push_back(trefChild.fourcc);
                    }

                if(!trefFound)
                  trackTypes.find(handlerType)->second.referenceTypes.push_back(FOURCC("main"));
              }
            }

      bool firstPredicate = true;

      for(auto& e : trackTypes)
      {
        int found = 0;

        for(auto referenceType : e.second.referenceTypes)
          if(referenceType == FOURCC("main"))
            found++;

        if(found != 1) // shall be exactly one
          firstPredicate = false;
      }

      bool secondPredicate = true;
      {
        uint16_t alternate_group = 0; // no information on possible relations to other tacks, dixit ISOBMFF

        for(auto& e : trackTypes)
        {
          for(auto referenceType : e.second.referenceTypes)
            if(referenceType != FOURCC("thmb") && referenceType != FOURCC("auxl"))
            {
              if(alternate_group)
              {
                if(alternate_group != e.second.alternateGroup)
                  secondPredicate = false;
              }
              else
              {
                if(e.second.alternateGroup) // found an alternate_group
                  alternate_group = e.second.alternateGroup;
              }
            }
        }

        if(!alternate_group)
          secondPredicate = false; // no alternate_group found, therefore can't form a group of alternates
      }

      if(!(firstPredicate ^ secondPredicate))
        out->error("There should be at most one single track of any given type (rule predicates: {%d, %d})", firstPredicate, secondPredicate);
    },
  },
  {
    "Section 8.6\n"
    "repeating edits may be used; either all tracks shall indicate a 'looping' edit, or none",
    [] (Box const& root, IReport* out)
    {
      int repeat = -1;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("edts"))
                  for(auto& edtsChild : trakChild.children)
                    if(edtsChild.fourcc == FOURCC("elst"))
                    {
                      for(auto& sym : edtsChild.syms)
                        if(!strcmp(sym.name, "flags"))
                        {
                          if(repeat == -1)
                            repeat = (sym.value & 1);

                          if(repeat != (sym.value & 1))
                            out->error("Either all tracks or none shall indicate an edit list repetition");
                        }
                    }
    }
  },
  {
    "Section 8.6\n"
    "the media_rate shall take either a value greater than 0 and up to and\n"
    "including 1 (forward play), or minus 1 (normal speed reverse play)", // -1, not 0, or 1 ?
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("edts"))
                  for(auto& edtsChild : trakChild.children)
                    if(edtsChild.fourcc == FOURCC("elst"))
                      for(auto& sym : edtsChild.syms)
                        if(!strcmp(sym.name, "media_rate_integer"))
                        {
                          int16_t mediaRate = sym.value;

                          if(mediaRate != -1 && mediaRate != 1)
                            out->error("'elst' media rate shall be -1 or 1, found %d", mediaRate);
                        }
    }
  },
  {
    "Section 8.6\n"
    "when there are two 'media edits' one shall specify forward playback and the other reverse\n"
    "over the same media time-range", // MEDIA COMPOSITION
    [] (Box const& root, IReport* out)
    {
      std::vector<int16_t> mediaRates;
      std::vector<int64_t> editDurations, mediaTimes;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("edts"))
                  for(auto& edtsChild : trakChild.children)
                    if(edtsChild.fourcc == FOURCC("elst"))
                      for(auto& sym : edtsChild.syms)
                      {
                        if(!strcmp(sym.name, "media_rate_integer"))
                          mediaRates.push_back(sym.value);

                        if(!strcmp(sym.name, "edit_duration"))
                          editDurations.push_back(sym.value);

                        if(!strcmp(sym.name, "media_time"))
                          mediaTimes.push_back(sym.value);
                      }

      if(mediaRates.size() == 2)
      {
        assert(editDurations.size() == 2 && mediaTimes.size() == 2);

        if(mediaRates[0] != 1 || mediaRates[1] != -1)
          out->error("Two media edits found: media rates shall be { 1, -1 }, found { %d, %d }", mediaRates[0], mediaRates[1]);

        if((mediaTimes[0] != mediaTimes[1] + mediaRates[1] * editDurations[1])
           && (mediaTimes[0] + mediaRates[0] * editDurations[0] != mediaTimes[1]))
          out->error("Two media edits found: one shall specify forward and the other reverse playback");
      }
    }
  },
  {
    "Section 8.6\n"
    "if an edit list is used to specify reverse playback (a negative media_rate) of some media\n"
    "there shall be a sync sample (as defined by ISO/IEC 14496-12) at least every 4 frames, or\n"
    "more often",
    [] (Box const& root, IReport* out)
    {
      bool hasReversePlayback = false;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("moov"))
          for(auto& moovChild : box.children)
            if(moovChild.fourcc == FOURCC("trak"))
              for(auto& trakChild : moovChild.children)
                if(trakChild.fourcc == FOURCC("edts"))
                  for(auto& edtsChild : trakChild.children)
                    if(edtsChild.fourcc == FOURCC("elst"))
                      for(auto& sym : edtsChild.syms)
                        if(!strcmp(sym.name, "media_rate_integer"))
                          if(sym.value != -1)
                            hasReversePlayback = true;

      if(hasReversePlayback)
      {
        int64_t lastSync = -4;

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
                              if(stblChild.fourcc == FOURCC("stss"))
                                for(auto& sym : stblChild.syms)
                                  if(!strcmp(sym.name, "sample_number"))
                                  {
                                    if(sym.value - lastSync > 4)
                                      out->error("Found edit list for reverse playback: sync sample delta (%lld - %lld) is more than 4", sym.value, lastSync);

                                    lastSync = sym.value;
                                  }
      }
    }
  },
  {
    "Section 8.2.1\n"
    "MIAF image items shall use data_reference_index==0, i.e. the image data shall\n"
    "be contained in the same file as the MetaBox.",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iloc"))
              for(auto& sym : metaChild.syms)
                if(!strcmp(sym.name, "data_reference_index"))
                  if(sym.value)
                    out->error("data_reference_index shall be 0, found %lld", sym.value);
    }
  },
  {
    "Section 8.2.2\n"
    "External data references in the DataReferenceBox shall not be used for image\n"
    "sequences or videos that are required, conditionally required, or explicitly\n"
    "permitted by this document, i.e. those sequences/videos shall be self-contained",
    [] (Box const& root, IReport* out)
    {
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("dinf"))
              for(auto& dinfChild : metaChild.children)
                if(dinfChild.fourcc == FOURCC("dref"))
                  for(auto& drefChild : dinfChild.children)
                    if(drefChild.fourcc == FOURCC("url ") || drefChild.fourcc == FOURCC("urn "))
                      for(auto& sym : drefChild.syms)
                        if(!strcmp(sym.name, "flags"))
                          if(!(sym.value & 1))
                            out->error("dref content is not in the same file");
    }
  },
  {
    "Section 7.4.6\n"
    "for every sample of the master image sequence or video track, there shall be a\n"
    "sample of the alpha plane track with the same composition time.",
    [] (Box const& root, IReport* out)
    {
      bool found = false;

      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipco"))
                  for(auto& ipcoChild : iprpChild.children)
                    if(ipcoChild.fourcc == FOURCC("auxC"))
                    {
                      std::string auxType;

                      for(auto& sym : ipcoChild.syms)
                        if(!strcmp(sym.name, "aux_type"))
                        {
                          auxType.push_back((char)sym.value);

                          if(auxType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")
                            found = true;
                        }
                    }

      if(!found)
        return;

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
                                    if(sampleEntryChild.fourcc == FOURCC("auxi"))
                                    {
                                      std::string auxType;

                                      for(auto& sym : sampleEntryChild.syms)
                                        if(!strcmp(sym.name, "aux_track_type"))
                                        {
                                          auxType.push_back((char)sym.value);

                                          if(auxType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")
                                            found = true;
                                        }
                                    }

      if(!found)
        return;

      auto findTrackId = [] (Box const& root) -> uint32_t {
          for(auto& trakChild : root.children)
            if(trakChild.fourcc == FOURCC("tkhd"))
              for(auto& sym : trakChild.syms)
                if(!strcmp(sym.name, "track_ID"))
                  return (uint32_t)sym.value;

          return 0;
        };

      struct Track
      {
        uint32_t videoTrackId = 0, alphaPlaneTrackId = 0;
      };

      auto findAlphaTracks = [findTrackId] (Box const& root, IReport* out) -> std::vector<Track> {
          std::vector<Track> trackIds;

          for(auto& box : root.children)
            if(box.fourcc == FOURCC("moov"))
              for(auto& moovChild : box.children)
                if(moovChild.fourcc == FOURCC("trak"))
                {
                  // find the hldr
                  uint32_t handlerType = 0;

                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("mdia"))
                      for(auto& mdiaChild : trakChild.children)
                        if(mdiaChild.fourcc == FOURCC("hdlr"))
                          for(auto& sym : mdiaChild.syms)
                            if(!strcmp(sym.name, "handler_type"))
                              handlerType = (uint32_t)sym.value;

                  // find tref track_id
                  uint32_t trackId = 0;

                  for(auto& trakChild : moovChild.children)
                    if(trakChild.fourcc == FOURCC("tref"))
                      for(auto& trefChild : trakChild.children)
                        if(trefChild.fourcc == FOURCC("auxl"))
                          for(auto& sym : trefChild.syms)
                            if(!strcmp(sym.name, "track_IDs"))
                            {
                              if(trackId)
                                out->error("Unexpected: found several 'tref' track_IDs (%u then %lld)", trackId, sym.value);

                              trackId = (uint32_t)sym.value;
                            }

                  if(handlerType == FOURCC("pict") && trackId)
                  {
                    auto id = findTrackId(moovChild);

                    if(id)
                      trackIds.push_back({ trackId, id });
                  }
                }

          return trackIds;
        };

      auto tracks = findAlphaTracks(root, out);

      for(auto& t : tracks)
      {
        auto getMediaTimes = [findTrackId] (Box const& root, uint32_t trackId) -> std::vector<Box const*> {
            std::vector<Box const*> mediaTimes;

            for(auto& box : root.children)
              if(box.fourcc == FOURCC("moov"))
                for(auto& moovChild : box.children)
                  if(moovChild.fourcc == FOURCC("trak"))
                    for(auto& trakChild : moovChild.children)
                    {
                      auto id = findTrackId(moovChild);

                      if(id != trackId)
                        continue;

                      if(trakChild.fourcc == FOURCC("mdia"))
                        for(auto& mdiaChild : trakChild.children)
                          if(mdiaChild.fourcc == FOURCC("minf"))
                            for(auto& minfChild : mdiaChild.children)
                              if(minfChild.fourcc == FOURCC("stbl"))
                                for(auto& stblChild : minfChild.children)
                                  if(stblChild.fourcc == FOURCC("stts"))
                                    mediaTimes.push_back(&stblChild);
                    }

            return mediaTimes;
          };

        auto mediaTimesMaster = getMediaTimes(root, t.videoTrackId);
        auto mediaTimesAlpha = getMediaTimes(root, t.alphaPlaneTrackId);

        struct EditListInfo
        {
          uint32_t edit_duration = 0;
          uint32_t media_time = 0;
          uint16_t media_rate_integer = 0;
        };

        auto getEditLists = [findTrackId] (Box const& root, uint32_t trackId) -> std::vector<Box const*> {
            std::vector<Box const*> editLists;

            for(auto& box : root.children)
              if(box.fourcc == FOURCC("moov"))
                for(auto& moovChild : box.children)
                  if(moovChild.fourcc == FOURCC("trak"))
                    for(auto& trakChild : moovChild.children)
                    {
                      auto id = findTrackId(moovChild);

                      if(id != trackId)
                        continue;

                      if(trakChild.fourcc == FOURCC("edts"))
                        for(auto& edtsChild : trakChild.children)
                          if(edtsChild.fourcc == FOURCC("elst"))
                            editLists.push_back(&edtsChild);
                    }

            return editLists;
          };

        auto editListsMaster = getEditLists(root, t.videoTrackId);
        auto editListsAlpha = getEditLists(root, t.alphaPlaneTrackId);

        auto compareBoxVector = [] (std::vector<Box const*>& a, std::vector<Box const*>& b)
          {
            if(a.size() != b.size())
              return false;

            for(size_t i = 0; i < a.size(); ++i)
              if(!boxEqual(a[i], b[i]))
                return false;

            return true;
          };

        if(!compareBoxVector(mediaTimesMaster, mediaTimesAlpha) || !compareBoxVector(editListsMaster, editListsAlpha))
          out->error("Composition times for trackId=%u different from alpha plane trackId=%u", t.videoTrackId, t.alphaPlaneTrackId);
      }
    }
  },
  {
    "Section 7.3.9\n"
    "All transformative properties associated with coded and derived images required\n"
    "or conditionally required by this document shall be marked as essential, and\n"
    "shall be from the set that are permitted by this document or the applicable\n"
    "profile. No other essential transformative property shall be associated with\n"
    "such images.",
    [] (Box const& root, IReport* out)
    {
      // TODO: know what a "transformative property" is
      for(auto& box : root.children)
        if(box.fourcc == FOURCC("meta"))
          for(auto& metaChild : box.children)
            if(metaChild.fourcc == FOURCC("iprp"))
              for(auto& iprpChild : metaChild.children)
                if(iprpChild.fourcc == FOURCC("ipma"))
                  for(auto& sym : iprpChild.syms)
                    if(!strcmp(sym.name, "essential"))
                      if(sym.value == 0)
                        out->error("All transformative properties shall be marked as essential");
    }
  },
};

static std::vector<RuleDesc> concat(const std::initializer_list<const std::initializer_list<RuleDesc>>& rules)
{
  std::vector<RuleDesc> v;

  for(auto& r : rules)
    v.insert(v.end(), r.begin(), r.end());

  return v;
}

extern const std::initializer_list<RuleDesc> getRulesAudio();
extern const std::initializer_list<RuleDesc> getRulesDerivations();
extern const std::initializer_list<RuleDesc> getRulesNumPixels();
extern const std::initializer_list<RuleDesc> getRulesBrands(const SpecDesc& spec);
extern const std::initializer_list<RuleDesc> getRulesProfiles(const SpecDesc& spec);

static const SpecDesc spec =
{
  "miaf",
  "MIAF (Multi-Image Application Format)\n"
  "MPEG-A part 22 - ISO/IEC 23000-22 - w18260 FDIS - Jan 2019",
  concat({ rulesGeneral, getRulesAudio(), getRulesDerivations(), getRulesNumPixels(), getRulesBrands(spec), getRulesProfiles(spec) }),
  nullptr,
};

static auto const registered = registerSpec(&spec);

