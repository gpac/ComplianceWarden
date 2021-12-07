%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x6D, 0x69, 0x66, 0x31 ; brand(32) ('mif1') 
    db 0x00, 0x00, 0x00, 0x00 ; version(32) 
    db 0x6D, 0x69, 0x66, 0x31 ; compatible_brand(32) ('mif1') 
    db 0x61, 0x76, 0x69, 0x66 ; compatible_brand(32) ('avif') 
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
        db 0x00 ; name(8) 
    hdlr_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x02 ; item_ID(16) 
    pitm_end:
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x01 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x04 ; offset_size(4) length_size(4) 
        db 0x40 ; base_offset_size(4) ('@') reserved1(4) ('@') 
        db 0x00, 0x02 ; item_count(16) 
        db 0x00, 0x02 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x00 ; extent_count(16) 
        db 0x00, 0x01 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        dd BE(mdat_start - ftyp_start + 8) ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x0, 0x01 ; extent_length(32) 
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
            db 0x00, 0x02 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x69, 0x64, 0x65, 0x6E ; item_type(32) ('iden') 
            db 0x00 ; item_name(8) 
        infe_end:
        infe2_start:
            dd BE(infe2_end - infe2_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x01 ; flags(24) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe2_end:
    iinf_end:
    iref_start:
        dd BE(iref_end - iref_start)
        dd "iref"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x0E ; box_size(32) 
        db 0x64, 0x69, 0x6D, 0x67 ; box_type(32) ('dimg') 
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
                db 0x00, 0x00, 0x08, 0x00 ; image_width(32) 
                db 0x00, 0x00, 0x03, 0x5A ; image_height(32) 
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
                db 0x00 ; (8) 
                db 0x00 ; (8) 
            pixi_end:
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
                db 0x28 ; seq_profile(3) ('(') seq_level_idx_0(5) ('(') 
                db 0x00 ; seq_tier_0(1) high_bitdepth(1) twelve_bit(1) monochrome(1) chroma_subsampling_x(1) chroma_subsampling_y(1) chroma_sample_position(2) 
                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
                 ; configOBUs(0) 
            av1C_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x00, 0x00, 0x03 ; entry_count(32) 
            db 0x00, 0x02 ; item_ID(16) 
            db 0x02 ; association_count(8) 
            db 0x01 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x04 ; association_count(8) 
            db 0x01 ; essential(1) property_index(7) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x84 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x04 ; association_count(8) 
            db 0x01 ; essential(1) property_index(7) 
            db 0x03 ; essential(1) property_index(7) 
            db 0x84 ; essential(1) property_index(7) 
            db 0x82 ; essential(1) property_index(7) 
        ipma_end:
    iprp_end:
meta_end:

mdat_start:
dd BE(mdat_end - mdat_start)
db "mdat"
db 0x00
mdat_end:
