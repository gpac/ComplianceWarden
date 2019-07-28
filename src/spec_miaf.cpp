#include "spec.h"
#include <cstring>
#include <functional>

static const SpecDesc spec =
{
  "miaf",
  "MIAF (Multi-Image Application Format)\n"
  "MPEG-A part 22 - ISO/IEC 23000-22 - w18260 FDIS - Jan 2019",
  {
    {
      "Section 3.13:\n"
      "The file-level MetaBox shall always be present (see 7.2.1.4).\n"
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
              out->error("The HandlerBox shall be the first contained box within the MetaBox.");
          }
      }
    },
    {
      "Section 7.2.1.2\n"
      "The FileTypeBox shall contain, in the compatible_brands list, "
      "the following (in any order): 'mif1' (specified in ISO/IEC 23008-12) "
      "[and] brand(s) identifying conformance to this document (specified in 10)."
      "[...]"
      "Files conforming to the general restrictions in clause 7 shall include "
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
          out->error("compatible_brands list shall contain 'miaf' (%s) and 'mif1' (%s).", strFound(foundMiaf), strFound(foundMif1));
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
                out->error("MetaBox shall not contain a XMLBox.");

              if(metaChild.fourcc == FOURCC("bxml"))
                out->error("MetaBox shall not contain a BinaryXMLBox.");
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
                      out->error("The handler type for the MetaBox shall be 'pict'.");
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
        bool foundIref = false;
        int constructionMethod = -1;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
            {
              if(metaChild.fourcc == FOURCC("iloc"))
              {
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "construction_method"))
                    constructionMethod = field.value;
              }
              else if(metaChild.fourcc == FOURCC("iref"))
              {
                foundIref = true;
              }
            }

        if(constructionMethod == 1 && !foundIref)
          out->error("construction_method=1 on a coded image item");
      },
    },
    {
      "Section 7.2.1.8\n"
      "MIAF image items shall not reference any item protection",
      [] (Box const& root, IReport* out)
      {
        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
          {
            bool isMiafItem = false, isProtected = false;

            for(auto& metaChild : box.children)
            {
              if(metaChild.fourcc == FOURCC("ipro"))
                isProtected = true;
              else if(metaChild.fourcc == FOURCC("hdlr"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "handler_type"))
                  {
                    if(field.value == FOURCC("pict"))
                      isMiafItem = true;
                  }
            }

            if(isMiafItem && isProtected)
              out->error("MIAF image item shall not reference any item protection");
          }
      },
    },
    {
      "Section 7.2.1.9\n"
      "ItemInfoBox "
      "Version 0 or 1 of this box is required by ISO/IEC 23008-12",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("iinf"))
                for(auto& field : metaChild.syms)
                  if(!strcmp(field.name, "version"))
                  {
                    if(field.value == 0 || field.value == 1)
                      found = true;
                    else
                      out->error("Version 0 or 1 of ItemInfoBox is required");
                  }

        if(!found)
          out->error("ItemInfoBox is required");
      },
    },
    {
      "Section 7.3.2\n"
      "The primary item shall be a MIAF master image item.",
      [] (Box const& root, IReport* out)
      {
        bool found = false;

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("pitm"))
              {
                found = true;

                // for(auto& field : metaChild.syms)
                // if(!strcmp(field.name, "item_ID"))
                // itemId = field.value;
              }

        if(!found)
          out->error("PrimaryItemBox is required");

        // thumbnails
        // TODO: we need a real sample and to generate invalid* samples
        // out->error("MIAF master image shall not be a thumbnail image");

        for(auto& box : root.children)
          if(box.fourcc == FOURCC("meta"))
            for(auto& metaChild : box.children)
              if(metaChild.fourcc == FOURCC("ipco"))
                for(auto& metaChild : box.children)
                  if(metaChild.fourcc == FOURCC("auxC"))
                    out->error("MIAF master image shall not be an auxiliary image");
      },
    },
    {
      "Section 7.3.6.3\n"
      "Every image item be associated with a Image spatial extents property",
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
            for(auto& field : clli.syms)
              if(strcmp(field.name, "max_content_light_level") && strcmp(field.name, "max_pic_average_light_level"))
                out->error("Invalid 'clli' field \"%s\" (value=%lld).", field.name, field.value);
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
                                  if(stsdChild.fourcc == FOURCC("avc1"))
                                    for(auto& avc1Child : stsdChild.children)
                                      if(avc1Child.fourcc == FOURCC("clli"))
                                      {
                                        auto& clli = avc1Child;
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
                  out->error("Invalid 'clli' position (parent is '%c%c%c%c').",
                             (parent.fourcc >> 24) & 0xff, (parent.fourcc >> 16) & 0xff,
                             (parent.fourcc >> 8) & 0xff, (parent.fourcc >> 0) & 0xff);
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
            for(auto& field : mdcv.syms)
              if(strcmp(field.name, "display_primaries_x_0") && strcmp(field.name, "display_primaries_y_0")
                 && strcmp(field.name, "display_primaries_x_1") && strcmp(field.name, "display_primaries_y_1")
                 && strcmp(field.name, "display_primaries_x_2") && strcmp(field.name, "display_primaries_y_2")
                 && strcmp(field.name, "white_point_x") && strcmp(field.name, "white_point_y")
                 && strcmp(field.name, "max_display_mastering_luminance") && strcmp(field.name, "min_display_mastering_luminance"))
                out->error("Invalid 'mdcv' field \"%s\" (value=%lld).", field.name, field.value);
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
                                  if(stsdChild.fourcc == FOURCC("avc1"))
                                    for(auto& avc1Child : stsdChild.children)
                                      if(avc1Child.fourcc == FOURCC("mdcv"))
                                      {
                                        auto& mdcv = avc1Child;
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
                  out->error("Invalid 'mdcv' position (parent is '%c%c%c%c').",
                             (parent.fourcc >> 24) & 0xff, (parent.fourcc >> 16) & 0xff,
                             (parent.fourcc >> 8) & 0xff, (parent.fourcc >> 0) & 0xff);
                }
              }
              else
                parse(box);
          };

        parse(root);
      },
    },
  },
  nullptr,
};

static auto const registered = registerSpec(&spec);

