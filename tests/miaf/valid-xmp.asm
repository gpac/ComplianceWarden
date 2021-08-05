%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x68, 0x65, 0x69, 0x63 ; "major_brand(32)" ('heic') 
    db 0x00, 0x00, 0x00, 0x00 ; "minor_version(32)" 
    db 0x6D, 0x69, 0x66, 0x31 ; "compatible_brand(32)" ('mif1') 
    db 0x68, 0x65, 0x69, 0x63 ; "compatible_brand(32)" ('heic') 
    db 0x6D, 0x69, 0x61, 0x66 ; "compatible_brand(32)" ('miaf') 
ftyp_end:
meta_start:
    dd BE(meta_end - meta_start)
    dd "meta"
    db 0x00 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    hdlr_start:
        dd BE(hdlr_end - hdlr_start)
        dd "hdlr"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x00, 0x00, 0x00 ; "pre_defined(32)" 
        db 0x70, 0x69, 0x63, 0x74 ; "handler_type(32)" ('pict') 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved1(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved2(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved3(32)" 
        db 0x00 ; "name(8)" 
        db 0x00 ; "name(8)" 
    hdlr_end:
    dinf_start:
        dd BE(dinf_end - dinf_start)
        dd "dinf"
        dref_start:
            dd BE(dref_end - dref_start)
            dd "dref"
            db 0x00 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
            url_start:
                dd BE(url_end - url_start)
                dd "url "
                db 0x00 ; "version(8)" 
                db 0x00, 0x00, 0x01 ; "flags(24)" 
            url_end:
        dref_end:
    dinf_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x01 ; "item_ID(16)" 
    pitm_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x02 ; "entry_count(16)" 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x01 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x68, 0x76, 0x63, 0x31 ; "item_type(32)" ('hvc1') 
            db 0x00 ; "item_name(8)" 
        infe_end:
        infe2_start:
            dd BE(infe2_end - infe2_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x01 ; "flags(24)" 
            db 0x00, 0x02 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x6D, 0x69, 0x6D, 0x65 ; "item_type(32)" ('mime') 
            db 0x00 ; "item_name(8)" 
            db 0x61 ; "content_type(8)" ('a') 
            db 0x70 ; "content_type(8)" ('p') 
            db 0x70 ; "content_type(8)" ('p') 
            db 0x6C ; "content_type(8)" ('l') 
            db 0x69 ; "content_type(8)" ('i') 
            db 0x63 ; "content_type(8)" ('c') 
            db 0x61 ; "content_type(8)" ('a') 
            db 0x74 ; "content_type(8)" ('t') 
            db 0x69 ; "content_type(8)" ('i') 
            db 0x6F ; "content_type(8)" ('o') 
            db 0x6E ; "content_type(8)" ('n') 
            db 0x2F ; "content_type(8)" ('/') 
            db 0x72 ; "content_type(8)" ('r') 
            db 0x64 ; "content_type(8)" ('d') 
            db 0x66 ; "content_type(8)" ('f') 
            db 0x2B ; "content_type(8)" ('+') 
            db 0x78 ; "content_type(8)" ('x') 
            db 0x6D ; "content_type(8)" ('m') 
            db 0x6C ; "content_type(8)" ('l') 
            db 0x00 ; "content_type(8)" 
        infe2_end:
    iinf_end:
    iref_start:
        dd BE(iref_end - iref_start)
        dd "iref"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x00, 0x00, 0x0E ; "box_size(32)" 
        db 0x63, 0x64, 0x73, 0x63 ; "box_type(32)" ('cdsc') 
        db 0x00, 0x02 ; "from_item_ID(16)" 
        db 0x00, 0x01 ; "reference_count(16)" 
        db 0x00, 0x01 ; "to_item_ID(16)" 
    iref_end:
    iprp_start:
        dd BE(iprp_end - iprp_start)
        dd "iprp"
        ipco_start:
            dd BE(ipco_end - ipco_start)
            dd "ipco"
            hvcC_start:
                dd BE(hvcC_end - hvcC_start)
                dd "hvcC"
                db 0x01 ; "configurationVersion(8)" 
                db 0x03 ; "general_profile_space(2)" "general_tier_flag(1)" "general_profile_idc(5)" 
                db 0x00, 0x00, 0x00, 0x0E ; "general_profile_compatibility_flags(32)" 
                db 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00 ; "general_constraint_indicator_flags(48)" 
                db 0x5A ; "general_level_idc(8)" ('Z') 
                db 0xF0, 0x00 ; "reserved(4)" "min_spatial_segmentation_idc(12)" 
                db 0xFC ; "reserved(6)" "parallelismType(2)" 
                db 0xFD ; "reserved(6)" "chroma_format_idc(2)" 
                db 0xF8 ; "reserved(5)" "bit_depth_luma_minus8(3)" 
                db 0xF8 ; "reserved(5)" "bit_depth_chroma_minus8(3)" 
                db 0x00, 0x00 ; "avgFrameRate(16)" 
                db 0x0F ; "constantFrameRate(2)" "numTemporalLayers(3)" "temporalIdNested(1)" "lengthSizeMinusOne(2)" 
                db 0x03 ; "numOfArrays(8)" 
                db 0xA0 ; "array_completeness(1)" "reserved(1)" "NAL_unit_type(6)" 
                db 0x00, 0x01 ; "numNalus(16)" 
                db 0x00, 0x17 ; "nalUnitLength(16)" 
                db 0x40 ; "nalUnit(8)" ('@') 
                db 0x01 ; "nalUnit(8)" 
                db 0x0C ; "nalUnit(8)" 
                db 0x01 ; "nalUnit(8)" 
                db 0xFF ; "nalUnit(8)" 
                db 0xFF ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x70 ; "nalUnit(8)" ('p') 
                db 0x00 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0xB0 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x5A ; "nalUnit(8)" ('Z') 
                db 0x70 ; "nalUnit(8)" ('p') 
                db 0x24 ; "nalUnit(8)" ('$') 
                db 0xA1 ; "array_completeness(1)" "reserved(1)" "NAL_unit_type(6)" 
                db 0x00, 0x01 ; "numNalus(16)" 
                db 0x00, 0x1F ; "nalUnitLength(16)" 
                db 0x42 ; "nalUnit(8)" ('B') 
                db 0x01 ; "nalUnit(8)" 
                db 0x01 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x70 ; "nalUnit(8)" ('p') 
                db 0x00 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0xB0 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x03 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x5A ; "nalUnit(8)" ('Z') 
                db 0xA0 ; "nalUnit(8)" 
                db 0x02 ; "nalUnit(8)" 
                db 0x00 ; "nalUnit(8)" 
                db 0x80 ; "nalUnit(8)" 
                db 0x20 ; "nalUnit(8)" (' ') 
                db 0x16 ; "nalUnit(8)" 
                db 0x77 ; "nalUnit(8)" ('w') 
                db 0x92 ; "nalUnit(8)" 
                db 0x44 ; "nalUnit(8)" ('D') 
                db 0x8A ; "nalUnit(8)" 
                db 0x26 ; "nalUnit(8)" ('&') 
                db 0xC0 ; "nalUnit(8)" 
                db 0x10 ; "nalUnit(8)" 
                db 0xA2 ; "array_completeness(1)" "reserved(1)" "NAL_unit_type(6)" 
                db 0x00, 0x01 ; "numNalus(16)" 
                db 0x00, 0x08 ; "nalUnitLength(16)" 
                db 0x44 ; "nalUnit(8)" ('D') 
                db 0x01 ; "nalUnit(8)" 
                db 0xC0 ; "nalUnit(8)" 
                db 0x63 ; "nalUnit(8)" ('c') 
                db 0x49 ; "nalUnit(8)" ('I') 
                db 0x30 ; "nalUnit(8)" ('0') 
                db 0x53 ; "nalUnit(8)" ('S') 
                db 0x24 ; "nalUnit(8)" ('$') 
            hvcC_end:
            ispe_start:
                dd BE(ispe_end - ispe_start)
                dd "ispe"
                db 0x00 ; "version(8)" 
                db 0x00, 0x00, 0x00 ; "flags(24)" 
                db 0x00, 0x00, 0x04, 0x00 ; "image_width(32)" 
                db 0x00, 0x00, 0x02, 0x00 ; "image_height(32)" 
            ispe_end:
            pixi_start:
                dd BE(pixi_end - pixi_start)
                dd "pixi"
            pixi_end:
            clap_start:
                dd BE(clap_end - clap_start)
                dd "clap"
                db 0x00, 0x00, 0x00, 0x00 ; "cleanApertureWidthN(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "cleanApertureWidthD(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "cleanApertureHeightN(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "cleanApertureHeightD(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "horizOffN(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "horizOffD(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "vertOffN(32)"
                db 0x00, 0x00, 0x00, 0x00 ; "vertOffD(32)"
            clap_end:
            imir_start:
                dd BE(imir_end - imir_start)
                dd "imir"
            imir_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
            db 0x00, 0x01 ; "item_ID(16)" 
            db 0x05 ; "association_count(8)" 
            db 0x81 ; "essential(1)" "property_index(7)" 
            db 0x02 ; "essential(1)" "property_index(7)" 
            db 0x03 ; "essential(1)" "property_index(7)" 
            db 0x84 ; "essential(1)" "property_index(7)" 
            db 0x85 ; "essential(1)" "property_index(7)" 
        ipma_end:
    iprp_end:
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x44 ; "offset_size(4)" ('D') "length_size(4)" ('D') 
        db 0x00 ; "base_offset_size(4)" "reserved1(4)" 
        db 0x00, 0x02 ; "item_count(16)" 
        db 0x00, 0x01 ; "item_ID(16)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        dd BE(mdat_start - ftyp_start + 8) ; "extent_offset(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "extent_length(32)" 
        db 0x00, 0x02 ; "item_ID(16)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        dd BE(mdat_start - ftyp_start + 8)  ; "extent_offset(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "extent_length(32)" 
    iloc_end:
meta_end:
mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
    db 0x00
mdat_end:

; vim: syntax=nasm
