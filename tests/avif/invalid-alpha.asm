%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x61, 0x76, 0x69, 0x66 ; brand(32) ('avif') 
    db 0x00, 0x00, 0x00, 0x00 ; version(32) 
    db 0x61, 0x76, 0x69, 0x66 ; compatible_brand(32) ('avif') 
    db 0x6D, 0x69, 0x66, 0x31 ; compatible_brand(32) ('mif1') 
    db 0x6D, 0x69, 0x61, 0x66 ; compatible_brand(32) ('miaf') 
    db 0x4D, 0x41, 0x31, 0x42 ; compatible_brand(32) ('MA1B') 
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
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x44 ; offset_size(4) ('D') length_size(4) ('D') 
        db 0x00 ; base_offset_size(4) reserved1(4) 
        db 0x00, 0x02 ; item_count(16) 
        db 0x00, 0x01 ; item_ID(16) 
        db 0x00, 0x00 ; data_reference_index(16) 
         ; base_offset(0) 
        db 0x00, 0x01 ; extent_count(16) 
        dd BE(mdat_start - ftyp_start + 8) ; extent_offset(32) 
        db 0x00, 0x00, 0x00, 0x08 ; extent_length(32) 
        db 0x00, 0x02 ; item_ID(16) 
        db 0x00, 0x00 ; data_reference_index(16) 
         ; base_offset(0) 
        db 0x00, 0x01 ; extent_count(16) 
        dd BE(mdat_start - ftyp_start + 8 + 8) ; extent_offset(32) 
        db 0x00, 0x00, 0x00, 0x0B ; extent_length(32) 
    iloc_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x02 ; entry_count(16) 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x43 ; item_name(8) ('c') 
            db 0x6F ; item_name(8) ('o') 
            db 0x6C ; item_name(8) ('l') 
            db 0x6F ; item_name(8) ('o') 
            db 0x72 ; item_name(8) ('r') 
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
            db 0x41 ; item_name(8) ('a') 
            db 0x6C ; item_name(8) ('l') 
            db 0x70 ; item_name(8) ('p') 
            db 0x68 ; item_name(8) ('h') 
            db 0x61 ; item_name(8) ('a') 
            db 0x00 ; item_name(8) 
        infe2_end:
    iinf_end:
    iref_start:
        dd BE(iref_end - iref_start)
        dd "iref"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x0E ; box_size(32) 
        db 0x61, 0x75, 0x78, 0x6C ; box_type(32) ('auxl') 
        db 0x00, 0x02 ; from_item_ID(16) 
        db 0x00, 0x01 ; reference_count(16) 
        db 0x00, 0x01 ; to_item_ID(16) 
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
                db 0x00, 0x00, 0x02, 0x00 ; image_width(32) 
                db 0x00, 0x00, 0x02, 0x00 ; image_height(32) 
            ispe_end:
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
            av1C_start:
                dd BE(av1C_end - av1C_start)
                dd "av1C"
                db 0x81 ; marker(1) version(7) 
                db 0x21 ; seq_profile(3) ('!') seq_level_idx_0(5) ('!') 
                db 0x00 ; seq_tier_0(1) high_bitdepth(1) twelve_bit(1) monochrome(1) chroma_subsampling_x(1) chroma_subsampling_y(1) chroma_sample_position(2) 
                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
                 ; configOBUs(0) 
            av1C_end:
            colr_start:
                dd BE(colr_end - colr_start)
                dd "colr"
                db 0x6E ; (8) ('n') 
                db 0x63 ; (8) ('c') 
                db 0x6C ; (8) ('l') 
                db 0x78 ; (8) ('x') 
                db 0x00 ; (8) 
                db 0x02 ; (8) 
                db 0x00 ; (8) 
                db 0x02 ; (8) 
                db 0x00 ; (8) 
                db 0x06 ; (8) 
                db 0x80 ; (8) 
            colr_end:
            colr2_start:
                dd BE(colr2_end - colr2_start)
                dd "colr"
                db 0x6E ; (8) ('n') 
                db 0x63 ; (8) ('c') 
                db 0x6C ; (8) ('l') 
                db 0x78 ; (8) ('x') 
                db 0x00 ; (8) 
                db 0x02 ; (8) 
                db 0x00 ; (8) 
                db 0x02 ; (8) 
                db 0x00 ; (8) 
                db 0x06 ; (8) 
                db 0x80 ; (8) 
            colr2_end:
            ispe2_start:
                dd BE(ispe2_end - ispe2_start)
                dd "ispe"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x00, 0x00, 0x02, 0x00 ; image_width(32) 
                db 0x00, 0x00, 0x02, 0x00 ; image_height(32) 
            ispe2_end:
            pixi2_start:
                dd BE(pixi2_end - pixi2_start)
                dd "pixi"
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x01 ; (8) 
                db 0x08 ; (8) 
            pixi2_end:
            av1C2_start:
                dd BE(av1C2_end - av1C2_start)
                dd "av1C"
                db 0x81 ; marker(1) version(7) 
                db 0x01 ; seq_profile(3) seq_level_idx_0(5) 
                db 0x1C ; seq_tier_0(1) high_bitdepth(1) twelve_bit(1) monochrome(1) chroma_subsampling_x(1) chroma_subsampling_y(1) chroma_sample_position(2) 
                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
                 ; configOBUs(0) 
            av1C2_end:
            auxC_start:
                dd BE(auxC_end - auxC_start)
                dd "auxC"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x75 ; aux_type(8) ('u') 
                db 0x72 ; aux_type(8) ('r') 
                db 0x6E ; aux_type(8) ('n') 
                db 0x3A ; aux_type(8) (':') 
                db 0x6D ; aux_type(8) ('m') 
                db 0x70 ; aux_type(8) ('p') 
                db 0x65 ; aux_type(8) ('e') 
                db 0x67 ; aux_type(8) ('g') 
                db 0x3A ; aux_type(8) (':') 
                db 0x6D ; aux_type(8) ('m') 
                db 0x70 ; aux_type(8) ('p') 
                db 0x65 ; aux_type(8) ('e') 
                db 0x67 ; aux_type(8) ('g') 
                db 0x42 ; aux_type(8) ('B') 
                db 0x3A ; aux_type(8) (':') 
                db 0x63 ; aux_type(8) ('c') 
                db 0x69 ; aux_type(8) ('i') 
                db 0x63 ; aux_type(8) ('c') 
                db 0x70 ; aux_type(8) ('p') 
                db 0x3A ; aux_type(8) (':') 
                db 0x73 ; aux_type(8) ('s') 
                db 0x79 ; aux_type(8) ('y') 
                db 0x73 ; aux_type(8) ('s') 
                db 0x74 ; aux_type(8) ('t') 
                db 0x65 ; aux_type(8) ('e') 
                db 0x6D ; aux_type(8) ('m') 
                db 0x73 ; aux_type(8) ('s') 
                db 0x3A ; aux_type(8) (':') 
                db 0x61 ; aux_type(8) ('a') 
                db 0x75 ; aux_type(8) ('u') 
                db 0x78 ; aux_type(8) ('x') 
                db 0x69 ; aux_type(8) ('i') 
                db 0x6C ; aux_type(8) ('l') 
                db 0x69 ; aux_type(8) ('i') 
                db 0x61 ; aux_type(8) ('a') 
                db 0x72 ; aux_type(8) ('r') 
                db 0x79 ; aux_type(8) ('y') 
                db 0x3A ; aux_type(8) (':') 
                db 0x61 ; aux_type(8) ('a') 
                db 0x6C ; aux_type(8) ('l') 
                db 0x70 ; aux_type(8) ('p') 
                db 0x68 ; aux_type(8) ('h') 
                db 0x61 ; aux_type(8) ('a') 
                db 0x00 ; aux_type(8) 
            auxC_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x00, 0x00, 0x02 ; entry_count(32) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x05 ; association_count(8) 
            db 0x01 ; essential(1) property_index(7) 
            db 0x02 ; essential(1) property_index(7) 
            db 0x88 ; essential(1) property_index(7) 
            db 0x04 ; essential(1) property_index(7) 
            db 0x05 ; essential(1) property_index(7) 
            db 0x00, 0x02 ; item_ID(16) 
            db 0x04 ; association_count(8) 
            db 0x06 ; essential(1) property_index(7) 
            db 0x07 ; essential(1) property_index(7) 
            db 0x83 ; essential(1) property_index(7) 
            db 0x09 ; essential(1) property_index(7) 
        ipma_end:
    iprp_end:
meta_end:
mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
    db 0x0A ; (8) 
    db 0x06 ; (8) 
    db 0x18 ; (8) 
    db 0x62 ; (8) ('b') 
    db 0x3F ; (8) ('?') 
    db 0xFF ; (8) 
    db 0xFE ; (8) 
    db 0x95 ; (8) 
    db 0x0A ; (8) 
    db 0x09 ; (8) 
    db 0x38 ; (8) ('8') 
    db 0x62 ; (8) ('b') 
    db 0x3F ; (8) ('?') 
    db 0xFF ; (8) 
    db 0xFE ; (8) 
    db 0x90 ; (8) 
    db 0x20 ; (8) (' ') 
    db 0x20 ; (8) (' ') 
    db 0x69 ; (8) ('i') 
mdat_end:

; vim: syntax=nasm
