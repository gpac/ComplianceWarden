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
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x04 ; offset_size(4) length_size(4) 
        db 0x40 ; base_offset_size(4) ('@') reserved1(4) ('@') 
        db 0x00, 0x01 ; item_count(16) 
        db 0x00, 0x01 ; item_ID(16) 
        db 0x00, 0x00 ; data_reference_index(16) 
        db 0x00, 0x00, 0x01, 0x16 ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0xF6, 0xB5 ; extent_length(32) 
    iloc_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x01 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x00 ; item_name(8) 
        infe_end:
    iinf_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x01 ; item_ID(16) 
    pitm_end:
    iprp_start:
        dd BE(iprp_end - iprp_start)
        dd "iprp"
        ipco_start:
            dd BE(ipco_end - ipco_start)
            dd "ipco"
            pasp_start:
                dd BE(pasp_end - pasp_start)
                dd "pasp"
                db 0x00, 0x00, 0x00, 0x01 ; hSpacing(32) 
                db 0x00, 0x00, 0x00, 0x01 ; vSpacing(32) 
            pasp_end:
            ispe_start:
                dd BE(ispe_end - ispe_start)
                dd "ispe"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x00, 0x00, 0x04, 0xB4 ; image_width(32) 
                db 0x00, 0x00, 0x03, 0x20 ; image_height(32) 
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
                db 0x05 ; seq_profile(3) seq_level_idx_0(5) 
                db 0x0C ; seq_tier_0(1) high_bitdepth(1) twelve_bit(1) monochrome(1) chroma_subsampling_x(1) chroma_subsampling_y(1) chroma_sample_position(2) 
                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
                 ; configOBUs(0) 
                 ; obu(0) 
                db 0x0A ; forbidden(1) obu_type(4) obu_extension_flag(1) obu_has_size_field(1) obu_reserved_1bit(1) 
                db 0x07 ; leb128_byte(8) 
                 ; seqhdr(0) 
                db 0x19, 0x6A, 0x65, 0x9E, 0x3F ; seq_profile(3) still_picture(1) reduced_still_picture_header(1) timing_info_present_flag(0) decoder_model_info_present_flag(0) initial_display_delay_present_flag(0) operating_points_cnt_minus_1(0) operating_point_idc_0(0) seq_level_idx_0(5) seq_tier_0(0) decoder_model_present_for_this_op_0(0) initial_display_delay_present_for_this_op_0(0) frame_width_bits_minus_1(4) frame_height_bits_minus_1(4) max_frame_width_minus_1(11) max_frame_height_minus_1(10) use_128x128_superblock(1) 
                db 0xC8 ; enable_filter_intra(1) enable_intra_edge_filter(1) enable_interintra_compound(0) enable_masked_compound(0) enable_warped_motion(0) enable_dual_filter(0) enable_order_hint(0) enable_jnt_comp(0) enable_ref_frame_mvs(0) seq_force_screen_content_tools(0) seq_force_integer_mv(0) OrderHintBits(0) enable_superres(1) enable_cdef(1) enable_restoration(1) high_bitdepth(1) mono_chrome(1) color_description_present_flag(1) 
                db 0x04 ; color_range(1) chroma_sample_position(1) separate_uv_delta_q(1) film_grain_params_present(1) bits(4) 
                 ; /seqhdr(0) 
            av1C_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x04 ; association_count(8) 
            db 0x01 ; essential(1) property_index(7) 
            db 0x02 ; essential(1) property_index(7) 
            db 0x83 ; essential(1) property_index(7) 
            db 0x84 ; essential(1) property_index(7) 
        ipma_end:
    iprp_end:
meta_end:
mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
    db 0x0A ; (8) 
    db 0x07 ; (8) 
    db 0x19 ; (8) 
    db 0x6A ; (8) ('j') 
    db 0x65 ; (8) ('e') 
    db 0x9E ; (8) 
    db 0x3F ; (8) ('?') 
    db 0xC8 ; (8) 
    db 0x04 ; (8) 
    db 0x32 ; (8) ('2') 
    db 0x00 ; (8) 
mdat_end:

; vim: syntax=nasm
