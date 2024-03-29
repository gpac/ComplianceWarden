%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

_start:
     ; obu(0) 
    db 0x0F ; forbidden(1) obu_type(4) obu_extension_flag(1) obu_has_size_field(1) obu_reserved_1bit(1) 
    db 0xA1 ; temporal_id(3) spatial_id(2) extension_header_reserved_3bits(3) 
    db 0x5E ; leb128_byte(8) ('^') 
     ; seqhdr(0) 
    db 0xA0, 0x8A, 0x78 ; seq_profile(3) still_picture(1) reduced_still_picture_header(1) timing_info_present_flag(1) initial_display_delay_present_flag(1) operating_points_cnt_minus_1(5) operating_point_idc[i])(12) 
    db 0x4E, 0x99, 0x80, 0x46, 0xA4 ; seq_level_idx[i](5) seq_tier[i](1) operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](0) operating_point_idc[i])(12) seq_level_idx[i](5) 
     ; seq_tier[i](0) 
    db 0x08, 0x00, 0x00, 0x29, 0xB4, 0xC4, 0x50, 0x01, 0x6C, 0xC8, 0x00, 0x01, 0x90 ; operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](0) operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](0) operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](0) operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](1) operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](0) operating_point_idc[i])(12) seq_level_idx[i](5) seq_tier[i](1) 
    db 0x03 ; frame_width_bits_minus_1(4) frame_height_bits_minus_1(4) 
    db 0xCA ; max_frame_width_minus_1(1) max_frame_height_minus_1(4) frame_id_numbers_present_flag(1) use_128x128_superblock(1) enable_filter_intra(1) 
    db 0x58 ; enable_intra_edge_filter(1) ('X') enable_interintra_compound(1) ('X') enable_masked_compound(1) ('X') enable_warped_motion(1) ('X') enable_dual_filter(1) ('X') enable_order_hint(1) ('X') seq_choose_screen_content_tools(1) ('X') seq_force_screen_content_tools(1) ('X') 
    db 0x06, 0x36, 0xD0, 0x09 ; enable_superres(1) enable_cdef(1) enable_restoration(1) high_bitdepth(1) mono_chrome(1) color_description_present_flag(1) color_primaries(8) transfer_characteristics(8) matrix_coefficients(8) color_range(1) separate_uv_delta_q(1) 
    db 0x3E ; film_grain_params_present(1) ('>') bits(7) ('>') 
     ; /seqhdr(0) 
    db 0xF8 ; byte(8) 
    db 0x0D ; byte(8) 
    db 0xFB ; byte(8) 
    db 0x19 ; byte(8) 
    db 0xA2 ; byte(8) 
    db 0xB0 ; byte(8) 
    db 0x00 ; byte(8) 
    db 0x40 ; byte(8) ('@') 
    db 0x2C ; byte(8) (',') 
    db 0x0F ; byte(8) 
    db 0xE5 ; byte(8) 
    db 0x8D ; byte(8) 
    db 0xB0 ; byte(8) 
    db 0x6F ; byte(8) ('o') 
    db 0xDC ; byte(8) 
    db 0xD3 ; byte(8) 
    db 0x64 ; byte(8) ('d') 
    db 0xE2 ; byte(8) 
    db 0xFA ; byte(8) 
    db 0x8E ; byte(8) 
    db 0xEB ; byte(8) 
    db 0xC0 ; byte(8) 
    db 0x00 ; byte(8) 
    db 0x80 ; byte(8) 
    db 0x32 ; byte(8) ('2') 
    db 0x85 ; byte(8) 
    db 0x03 ; byte(8) 
    db 0x10 ; byte(8) 
    db 0x00 ; byte(8) 
    db 0x86 ; byte(8) 
    db 0x40 ; byte(8) ('@') 
    db 0xC0 ; byte(8) 
    db 0x02 ; byte(8) 
    db 0x00 ; byte(8) 
    db 0x01 ; byte(8) 
    db 0x02 ; byte(8) 
    db 0xA0 ; byte(8) 
    db 0x01 ; byte(8) 
    db 0x00 ; byte(8) 
    db 0x0A ; byte(8) 
    db 0x0A ; byte(8) 
_end:

; vim: syntax=nasm
