%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x6D, 0x69, 0x66, 0x31 ; major_brand(32) ('mif1') 
    db 0x00, 0x00, 0x00, 0x00 ; minor_version(32) 
    db 0x6D, 0x69, 0x66, 0x31 ; compatible_brand(32) ('mif1') 
    db 0x61, 0x76, 0x63, 0x69 ; compatible_brand(32) ('avci') 
    db 0x6D, 0x69, 0x61, 0x66 ; compatible_brand(32) ('miaf') 
ftyp_end:
meta_start:
    dd BE(meta_end - meta_start)
    dd "meta"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    hdlr_start:
        dd BE(hdlr_end - hdlr_start)
        dd "hdlr"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
        db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
        db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
        db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
        db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
        db 0x47 ; name(8) ('G') 
        db 0x50 ; name(8) ('P') 
        db 0x41 ; name(8) ('A') 
        db 0x43 ; name(8) ('C') 
        db 0x20 ; name(8) (' ') 
        db 0x70 ; name(8) ('p') 
        db 0x69 ; name(8) ('i') 
        db 0x63 ; name(8) ('c') 
        db 0x74 ; name(8) ('t') 
        db 0x20 ; name(8) (' ') 
        db 0x48 ; name(8) ('H') 
        db 0x61 ; name(8) ('a') 
        db 0x6E ; name(8) ('n') 
        db 0x64 ; name(8) ('d') 
        db 0x6C ; name(8) ('l') 
        db 0x65 ; name(8) ('e') 
        db 0x72 ; name(8) ('r') 
        db 0x00 ; name(8) 
    hdlr_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x01 ; item_ID(16) 
    pitm_end:
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x01 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x04 ; offset_size(4) length_size(4) 
        db 0x40 ; base_offset_size(4) ('@') reserved1(4) ('@') 
        db 0x00, 0x07 ; item_count(16) 
        db 0x00, 0x01 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat1_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x08 ; extent_length(32) 
        db 0x00, 0x02 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat2_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x0A ; extent_length(32) 
        db 0x00, 0x03 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat2_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x0A ; extent_length(32) 
        db 0x00, 0x04 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat2_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x0A ; extent_length(32) 
        db 0x00, 0x05 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat2_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0)  
        db 0x00, 0x00, 0x00, 0x0A ; extent_length(32) 
        db 0x00, 0x06 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat2_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x0A ; extent_length(32) 
        db 0x00, 0x07 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat2_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x0A ; extent_length(32) 
    iloc_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x07 ; entry_count(16) 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x67, 0x72, 0x69, 0x64 ; item_type(32) ('grid') 
            db 0x00 ; item_name(8) 
        infe_end:
        infe2_start:
            dd BE(infe2_end - infe2_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x02 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe2_end:
        infe3_start:
            dd BE(infe3_end - infe3_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x03 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe3_end:
        infe4_start:
            dd BE(infe4_end - infe4_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x04 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe4_end:
        infe5_start:
            dd BE(infe5_end - infe5_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x05 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe5_end:
        infe6_start:
            dd BE(infe6_end - infe6_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x06 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe6_end:
        infe7_start:
            dd BE(infe7_end - infe7_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x07 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe7_end:
    iinf_end:
    iref_start:
        dd BE(iref_end - iref_start)
        dd "iref"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x18 ; box_size(32) 
        db 0x64, 0x69, 0x6D, 0x67 ; box_type(32) ('dimg') 
        db 0x00, 0x01 ; from_item_ID(16) 
        db 0x00, 0x06 ; reference_count(16) 
        db 0x00, 0x02 ; to_item_ID(16) 
        db 0x00, 0x03 ; to_item_ID(16) 
        db 0x00, 0x04 ; to_item_ID(16) 
        db 0x00, 0x05 ; to_item_ID(16) 
        db 0x00, 0x06 ; to_item_ID(16) 
        db 0x00, 0x07 ; to_item_ID(16) 
    iref_end:
    iprp_start:
        dd BE(iprp_end - iprp_start)
        dd "iprp"
        ipco_start:
            dd BE(ipco_end - ipco_start)
            dd "ipco"
            ispe_start:
                dd BE(ispe_end - ispe_start)
                dd "ispe"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x00, 0x00, 0x00, 0xC0 ; image_width(32) 
                db 0x00, 0x00, 0x00, 0x80 ; image_height(32) 
            ispe_end:
            clap_start:
                dd BE(clap_end - clap_start)
                dd "clap"
                db 0x00, 0x00, 0x00, 0x80 ; cleanApertureWidthN(32) 
                db 0x00, 0x00, 0x00, 0x01 ; cleanApertureWidthD(32) 
                db 0x00, 0x00, 0x00, 0x20 ; cleanApertureHeightN(32) 
                db 0x00, 0x00, 0x00, 0x01 ; cleanApertureHeightD(32) 
                db 0x00, 0x00, 0x00, 0x00 ; horizOffN(32) 
                db 0x00, 0x00, 0x00, 0x01 ; horizOffD(32) 
                db 0x00, 0x00, 0x00, 0x00 ; vertOffN(32) 
                db 0x00, 0x00, 0x00, 0x01 ; vertOffD(32) 
            clap_end:
            ispe2_start:
                dd BE(ispe2_end - ispe2_start)
                dd "ispe"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x00, 0x00, 0x00, 0x40 ; image_width(32) 
                db 0x00, 0x00, 0x00, 0x40 ; image_height(32) 
            ispe2_end:
            pasp_start:
                dd BE(pasp_end - pasp_start)
                dd "pasp"
                db 0x00, 0x00, 0x00, 0x01 ; hSpacing(32) 
                db 0x00, 0x00, 0x00, 0x01 ; vSpacing(32) 
            pasp_end:
            av1C_start:
                dd BE(av1C_end - av1C_start)
                dd "av1C"
                db 0x81 ; marker(1) version(7) 
                db 0x20 ; seq_profile(3) (' ') seq_level_idx_0(5) (' ') 
                db 0x00 ; seq_tier_0(1) high_bitdepth(1) twelve_bit(1) monochrome(1) chroma_subsampling_x(1) chroma_subsampling_y(1) chroma_sample_position(2) 
                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
                 ; configOBUs(0) 
            av1C_end:
            pixi_start:
                dd BE(pixi_end - pixi_start)
                dd "pixi"
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x03 ; (8) 
                db 0x08 ; (8) 
                db 0x08 ; (8) 
                db 0x08 ; (8) 
            pixi_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x00, 0x00, 0x07 ; entry_count(32) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x02 ; association_count(8) 
            db 0x01 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
            db 0x00, 0x02 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x85 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
            db 0x00, 0x03 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x85 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
            db 0x00, 0x04 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x85 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
            db 0x00, 0x05 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x85 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
            db 0x00, 0x06 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x85 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
            db 0x00, 0x07 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x85 ; essential(1) property_index(7) 
            db 0x86 ; essential(1) property_index(7) 
        ipma_end:
    iprp_end:
meta_end:
free_start:
    dd BE(free_end - free_start)
    dd "free"
    db 0x49 ; (8) ('I') 
    db 0x73 ; (8) ('s') 
    db 0x6F ; (8) ('o') 
    db 0x4D ; (8) ('M') 
    db 0x65 ; (8) ('e') 
    db 0x64 ; (8) ('d') 
    db 0x69 ; (8) ('i') 
    db 0x61 ; (8) ('a') 
    db 0x20 ; (8) (' ') 
    db 0x46 ; (8) ('F') 
    db 0x69 ; (8) ('i') 
    db 0x6C ; (8) ('l') 
    db 0x65 ; (8) ('e') 
    db 0x20 ; (8) (' ') 
    db 0x50 ; (8) ('P') 
    db 0x72 ; (8) ('r') 
    db 0x6F ; (8) ('o') 
    db 0x64 ; (8) ('d') 
    db 0x75 ; (8) ('u') 
    db 0x63 ; (8) ('c') 
    db 0x65 ; (8) ('e') 
    db 0x64 ; (8) ('d') 
    db 0x20 ; (8) (' ') 
    db 0x77 ; (8) ('w') 
    db 0x69 ; (8) ('i') 
    db 0x74 ; (8) ('t') 
    db 0x68 ; (8) ('h') 
    db 0x20 ; (8) (' ') 
    db 0x47 ; (8) ('G') 
    db 0x50 ; (8) ('P') 
    db 0x41 ; (8) ('A') 
    db 0x43 ; (8) ('C') 
    db 0x20 ; (8) (' ') 
    db 0x31 ; (8) ('1') 
    db 0x2E ; (8) ('.') 
    db 0x31 ; (8) ('1') 
    db 0x2E ; (8) ('.') 
    db 0x30 ; (8) ('0') 
    db 0x2D ; (8) ('-') 
    db 0x44 ; (8) ('D') 
    db 0x45 ; (8) ('E') 
    db 0x56 ; (8) ('V') 
    db 0x2D ; (8) ('-') 
    db 0x72 ; (8) ('r') 
    db 0x65 ; (8) ('e') 
    db 0x76 ; (8) ('v') 
    db 0x33 ; (8) ('3') 
    db 0x31 ; (8) ('1') 
    db 0x34 ; (8) ('4') 
    db 0x2D ; (8) ('-') 
    db 0x67 ; (8) ('g') 
    db 0x65 ; (8) ('e') 
    db 0x62 ; (8) ('b') 
    db 0x34 ; (8) ('4') 
    db 0x31 ; (8) ('1') 
    db 0x64 ; (8) ('d') 
    db 0x32 ; (8) ('2') 
    db 0x61 ; (8) ('a') 
    db 0x32 ; (8) ('2') 
    db 0x35 ; (8) ('5') 
    db 0x2D ; (8) ('-') 
    db 0x69 ; (8) ('i') 
    db 0x6D ; (8) ('m') 
    db 0x61 ; (8) ('a') 
    db 0x67 ; (8) ('g') 
    db 0x65 ; (8) ('e') 
    db 0x5F ; (8) ('_') 
    db 0x6F ; (8) ('o') 
    db 0x76 ; (8) ('v') 
    db 0x65 ; (8) ('e') 
    db 0x72 ; (8) ('r') 
    db 0x6C ; (8) ('l') 
    db 0x61 ; (8) ('a') 
    db 0x79 ; (8) ('y') 
    db 0x00 ; (8) 
free_end:

; dimg
mdat1_start:
    dd BE(mdat1_end - mdat1_start)
    dd "mdat"
    db 0x00 ; derivation version
    db 0x00 ; dimg flags
    db 0x01 ; dimg rows_minus_one
    db 0x02 ; dimg columns_minus_one
    db 0x00, 0x00 ; dimg output_width
    db 0x00, 0x00 ; dimg output_height
mdat1_end:

; av1
mdat2_start:
    dd BE(mdat2_end - mdat2_start)
    dd "mdat"
    db 0x0A ; (8) 0000 1010
    db 0x06 ; (8) 
    db 0x38 ; (8) 0011 1000
    db 0x1D ; (8) 0001 1101
    db 0xF0 ; (8) 1111 0000
    db 0x20 ; (8) 0010 0000
    db 0x00 ; (8) 0000 0000
    db 0x20 ; (8) 0010 ;0 color config
    db 0x32 ; (8) ('2') ; sync frame
    db 0x00 ; (8) 
mdat2_end:

; vim: syntax=nasm
