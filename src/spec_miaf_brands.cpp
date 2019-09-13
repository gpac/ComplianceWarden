#include "spec.h"
#include "fourcc.h"

const std::initializer_list<RuleDesc> getRulesBrands()
{
  static const
  std::initializer_list<RuleDesc> rulesBrands =
  {
    {
      "Section 10.1\n"
      "A MIAF file shall use the filename extensions specified by HEIF to identify the\n"
      "presence of specific image coding formats",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out;
        // TODO
      }
    },
    {
      "Section 10.2\n"
      "'MiPr' in the compatible_brands in the FileTypeBox specifies that a file\n"
      "conforming to 'miaf' brand also conforms to the following constraints:\n"
      "- The MetaBox shall follow the FileTypeBox. There shall be no intervening boxes\n"
      "  between the FileTypeBox and the MetaBox except at most one BoxFileIndexBox.\n"
      "- The MediaDataBox shall not occur before the MetaBox.\n"
      "- At most one top-level FreeSpaceBox is allowed, which, if present, shall be\n"
      "  between the MetaBox and the MediaDataBox. There shall be no other top-level\n"
      "  FreeSpaceBox in the file.\n"
      "- The primary image item conforms to a MIAF profile.\n"
      "- There is at least one MIAF thumbnail image item present for the primary image\n"
      "  item and the coded data for the thumbnail image items precede in file order\n"
      "  the coded data for the primary item.\n"
      "- The maximum number of bytes from the beginning of the file to the last byte\n"
      "  of the coded data for at least one of the thumbnail images of the primary\n"
      "  item, or the primary item itself, is 128 000 bytes.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out;
        // TODO
      }
    },
    {
      "Section 10.3\n"
      "The presence of the animation MIAF application brand indication ('MiAn') in the\n"
      "FileTypeBox indicates that the file conforms to the following additional\n"
      "constraints:\n"
      "- There shall be:\n"
      "  * exactly one non-auxiliary video track or non-auxiliary image sequence track\n"
      "  * at most one auxiliary video track (which shall be an alpha plane track,\n"
      "    when present),\n"
      "  * at most one audio track, and\n"
      "  * no other media tracks.\n"
      "- The luma sample rate of each video track shall be less than or equal to\n"
      "  62 914 560 samples per second.\n"
      "- The constraints of subclause 8.6 apply.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out;
        // TODO
      }
    },
    {
      "Section 10.4\n"
      "A track indicated to conform to this brand shall be constrained as follows:\n"
      "- The track shall be an image sequence ('pict') track.\n"
      "- In the image sequence track, any single coded picture shall be decodable by\n"
      "  decoding a maximum of two coded pictures (i.e. the picture itself and at most\n"
      "  one reference), and these two coded pictures shall be a valid bitstream.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out;
        // TODO
      }
    },
    {
      "Section 10.5\n"
      "The presence of the brand 'MiAC' in the FileTypeBox indicates that the file\n"
      "conforms to the following additional constraints:\n"
      " - It conforms to the constraints of both the 'MiCm' and the 'MiAn' brands,\n"
      "   with the following constraints:\n"
      " - There is exactly one auxiliary alpha video track.\n"
      " - The non-auxiliary video track uses the 'vide' handler, and is not\n"
      "   pre-multiplied.\n"
      " - The tracks are fragmented.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out;
        // TODO
      }
    },
    {
      "Section 10.6\n"
      "The presence of the brand 'MiCm' in the FileTypeBox indicates that the file\n"
      "contains movie fragments that conform to the constraints of the 'cmfc' brand of\n"
      "ISO/IEC 23000-19, and the following additional constraints that apply when a\n"
      "MIAF file contains multiple tracks (e.g. a video or image sequence track and an\n"
      "auxiliary track):\n"
      " - each track, if considered separately, shall be a conforming CMAF track as\n"
      "   defined in ISO/IEC 23000-19. In other words, if all boxes related to the\n"
      "   other tracks were removed (e.g. file-level boxes such as MovieFragmentBoxes,\n"
      "   and boxes in the MovieBox such as the TrackBox or the TrackExtendsBox), the\n"
      "   content shall be conforming to the brand 'cmfc' defined in ISO/IEC 23000-19;\n"
      " - the set of CMAF tracks associated with all MIAF tracks (including any audio)\n"
      "   shall be of the same duration, within a tolerance of the longest CMAF\n"
      "   fragment duration of any CMAF track;\n"
      " - the set of CMAF tracks associated with the MIAF visual tracks shall have the\n"
      "   same duration, same number of fragments and fragments shall be time-aligned.\n"
      "   Fragments of the different CMAF tracks shall also be interleaved in the MIAF\n"
      "   file.",
      [] (Box const& root, IReport* out)
      {
        (void)root;
        (void)out;
        // TODO
      }
    },
  };
  return rulesBrands;
}

