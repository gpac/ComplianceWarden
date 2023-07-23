%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))
%define fourcc(a) db a

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"
db "isom"
dd BE(0x00)
db "mif1", "miaf"
db "av01"
ftyp_end:

moov_start:
dd BE(moov_end - moov_start)
fourcc("moov")
mvhd_start:
    dd BE(mvhd_end - mvhd_start)
    dd "mvhd"
mvhd_end:

trak_start:
dd BE(trak_end - trak_start)
fourcc("trak")

tkhd_start:
    dd BE(tkhd_end - tkhd_start)
    dd "tkhd"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x01 ; flags(24) 
    db 0xD7, 0xAE, 0x43, 0xC0 ; creation_time(32) 
    db 0xD8, 0x7E, 0xD7, 0x51 ; modification_time(32) 
    db 0x00, 0x00, 0x00, 0x02 ; track_ID(32) 
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
    db 0x00, 0x00, 0x09, 0x49 ; duration(32) 
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
    db 0x00, 0x00 ; layer(16) 
    db 0x00, 0x00 ; alternate_group(16) 
    db 0x00, 0x00 ; volume(16) 
    db 0x00, 0x00 ; reserved(16) 
    db 0x00, 0x01, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x01, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x40, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x09, 0x68 ; width(32) 
    db 0x00, 0x00, 0x06, 0x40 ; height(32) 
tkhd_end:

mdia_start:
dd BE(mdia_end - mdia_start)
fourcc("mdia")

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
db 0x00 ; version(8) 
db 0x00, 0x00, 0x00 ; flags(24) 
db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
db 0x00 ; name(8) 
hdlr_end:

mdhd_start:
    dd BE(mdhd_end - mdhd_start)
    dd "mdhd"
mdhd_end:

minf_start:
dd BE(minf_end - minf_start)
fourcc("minf")
vmhd_start:
    dd BE(vmhd_end - vmhd_start)
    dd "vmhd"
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x01 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
vmhd_end:
dinf_start:
    dd BE(dinf_end - dinf_start)
    dd "dinf"
    dref_start:
        dd BE(dref_end - dref_start)
        dd "dref"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
        url_start:
            dd BE(url_end - url_start)
            dd "url "
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x01 ; flags(24) 
        url_end:
    dref_end:
dinf_end:
stbl_start:
dd BE(stbl_end - stbl_start)
fourcc("stbl")
stsd_start:
dd BE(stsd_end - stsd_start)
fourcc("stsd")
dd BE(0)
dd BE(1) ; entry_count

; dd BE(1) ; entry_count
av01_start:
    dd BE(av01_end - av01_start)
    dd "av01"
    db 0x00 ; "reserved1(8)" 
    db 0x00 ; "reserved2(8)" 
    db 0x00 ; "reserved3(8)" 
    db 0x00 ; "reserved4(8)" 
    db 0x00 ; "reserved5(8)" 
    db 0x00 ; "reserved6(8)" 
    db 0x00, 0x01 ; "data_reference_index(16)" 
    db 0x00, 0x00 ; "pre_defined(16)" 
    db 0x00, 0x00 ; "reserved7(16)" 
    db 0x00, 0x00, 0x00, 0x00 ; "pre_defined1(32)" 
    db 0x00, 0x00, 0x00, 0x00 ; "pre_defined2(32)" 
    db 0x00, 0x00, 0x00, 0x00 ; "pre_defined3(32)" 
    db 0x04, 0xB4 ; "width(16)" 
    db 0x03, 0x20 ; "height(16)" 
    db 0x00, 0x48, 0x00, 0x00 ; "horizresolution(32)" 
    db 0x00, 0x48, 0x00, 0x00 ; "vertresolution(32)" 
    db 0x00, 0x00, 0x00, 0x00 ; "reserved8(32)" 
    db 0x00, 0x01 ; "frame_count(16)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00 ; "compressorname(8)" 
    db 0x00, 0x18 ; "depth(16)" 
    db 0xFF, 0xFF ; "pre_defined4(16)" 
    av1C3_start:
        dd BE(av1C3_end - av1C3_start)
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
    av1C3_end:
    ccst_start:
        dd BE(ccst_end - ccst_start)
        dd "ccst"
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x7C ; (8) ('|') 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
    ccst_end:
    btrt_start:
        dd BE(btrt_end - btrt_start)
        dd "btrt"
    btrt_end:
    clap_start:
      dd BE(clap_end - clap_start)
      dd "clap"
      dd BE(0)
      dd BE(0)
      dd BE(0)
      dd BE(0)
      dd BE(0)
      dd BE(0)
      dd BE(0)
      dd BE(0)
    clap_end:
    pasp_start:
      dd BE(pasp_end - pasp_start)
      dd "pasp"
      dd BE(90601); Hspacing
      dd BE(40000);Vspacing
    pasp_end:
av01_end:



stsd_end:

stts_start:
    dd BE(stts_end - stts_start)
    dd "stts"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
    db 0x00, 0x00, 0x00, 0x01 ; sample_count(32) 
    db 0x00, 0x00, 0x00, 0x01 ; sample_delta(32) 
stts_end:
stsc_start:
    dd BE(stsc_end - stsc_start)
    dd "stsc"
    db 0x00, 0x00, 0x00, 0x00
    db 0x00, 0x00, 0x00, 0x00
    db 0x00, 0x00, 0x00, 0x00
stsc_end:
stsz_start:
    dd BE(stsz_end - stsz_start)
    dd "stsz"
    db 0x00, 0x00, 0x00, 0x01
    db 0x00, 0x00, 0x00, 0x01
stsz_end:
stco_start:
    dd BE(stco_end - stco_start)
    dd "stco"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
    dd BE(mdat_start - ftyp_start + 8) ; chunk_offset(32) 
stco_end:

stbl_end:
minf_end:
mdia_end:
trak_end:

trak2_start:
dd BE(trak2_end - trak2_start)
fourcc("trak")

tkhd2_start:
    dd BE(tkhd2_end - tkhd2_start)
    dd "tkhd"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x01 ; flags(24) 
    db 0xD7, 0xAE, 0x43, 0xC0 ; creation_time(32) 
    db 0xD8, 0x7E, 0xD7, 0x51 ; modification_time(32) 
    db 0x00, 0x00, 0x00, 0x01 ; track_ID(32) 
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
    db 0x00, 0x00, 0x09, 0x49 ; duration(32) 
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
    db 0x00, 0x00 ; layer(16) 
    db 0x00, 0x00 ; alternate_group(16) 
    db 0x00, 0x00 ; volume(16) 
    db 0x00, 0x00 ; reserved(16) 
    db 0x00, 0x01, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x01, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x40, 0x00, 0x00, 0x00 ; matrix(32) 
    db 0x01, 0xE0, 0x00, 0x00 ; width(32) 
    db 0x01, 0x0E, 0x00, 0x00 ; height(32) 
tkhd2_end:

mdia2_start:
dd BE(mdia2_end - mdia2_start)
fourcc("mdia")

hdlr2_start:
dd BE(hdlr2_end - hdlr2_start)
db "hdlr"
db 0x00 ; version(8) 
db 0x00, 0x00, 0x00 ; flags(24) 
db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
db 0x00 ; name(8) 
hdlr2_end:

mdhd2_start:
    dd BE(mdhd2_end - mdhd2_start)
    dd "mdhd"
mdhd2_end:

minf2_start:
dd BE(minf2_end - minf2_start)
fourcc("minf")
vmhd2_start:
    dd BE(vmhd2_end - vmhd2_start)
    dd "vmhd"
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x01 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
vmhd2_end:
dinf2_start:
    dd BE(dinf2_end - dinf2_start)
    dd "dinf"
    dref2_start:
        dd BE(dref2_end - dref2_start)
        dd "dref"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
        url2_start:
            dd BE(url2_end - url2_start)
            dd "url "
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x01 ; flags(24) 
        url2_end:
    dref2_end:
dinf2_end:
stbl2_start:
dd BE(stbl2_end - stbl2_start)
fourcc("stbl")
stsd2_start:
dd BE(stsd2_end - stsd2_start)
fourcc("stsd")
dd BE(0)
dd BE(0) ; entry_count
stsd2_end:

stts2_start:
    dd BE(stts2_end - stts2_start)
    dd "stts"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
    db 0x00, 0x00, 0x00, 0x01 ; sample_count(32) 
    db 0x00, 0x00, 0x00, 0x01 ; sample_delta(32) 
stts2_end:
stsc2_start:
    dd BE(stsc2_end - stsc2_start)
    dd "stsc"
    db 0x00, 0x00, 0x00, 0x00
    db 0x00, 0x00, 0x00, 0x00
    db 0x00, 0x00, 0x00, 0x00
stsc2_end:
stsz2_start:
    dd BE(stsz2_end - stsz2_start)
    dd "stsz"
    db 0x00, 0x00, 0x00, 0x01
    db 0x00, 0x00, 0x00, 0x01
stsz2_end:
stco2_start:
    dd BE(stco2_end - stco2_start)
    dd "stco"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
    dd BE(mdat_start - ftyp_start + 8) ; chunk_offset(32) 
stco2_end:

stbl2_end:
minf2_end:
mdia2_end:
trak2_end:

moov_end:

mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
     ; obu(0) 
    db 0x12 ; forbidden(1) obu_type(4) obu_extension_flag(1) obu_has_size_field(1) obu_reserved_1bit(1) 
    db 0x00 ; leb128_byte(8) 
     ; obu(0) 
    db 0x0A ; forbidden(1) obu_type(4) obu_extension_flag(1) obu_has_size_field(1) obu_reserved_1bit(1) 
    db 0x0F ; leb128_byte(8) 
     ; seqhdr(0) 
    db 0x00, 0x00, 0x00 ; seq_profile(3) still_picture(1) reduced_still_picture_header(1) timing_info_present_flag(1) initial_display_delay_present_flag(1) operating_points_cnt_minus_1(5) operating_point_idc[i])(12) 
    db 0x6A, 0xEF, 0xFF, 0xE1, 0xBD ; seq_level_idx[i](5) seq_tier[i](1) frame_width_bits_minus_1(4) frame_height_bits_minus_1(4) max_frame_width_minus_1(12) max_frame_height_minus_1(12) frame_id_numbers_present_flag(1) use_128x128_superblock(1) 
    db 0xFF ; enable_filter_intra(1) enable_intra_edge_filter(1) enable_interintra_compound(1) enable_masked_compound(1) enable_warped_motion(1) enable_dual_filter(1) enable_order_hint(1) enable_jnt_comp(1) 
    db 0xF9 ; enable_ref_frame_mvs(1) seq_choose_screen_content_tools(1) seq_choose_integer_mv(1) order_hint_bits_minus_1(3) enable_superres(1) enable_cdef(1) 
    db 0xD0, 0x91, 0x00, 0x94 ; enable_restoration(1) high_bitdepth(1) mono_chrome(1) color_description_present_flag(1) color_primaries(8) transfer_characteristics(8) matrix_coefficients(8) color_range(1) chroma_sample_position(2) separate_uv_delta_q(1) 
    db 0x40 ; film_grain_params_present(1) ('@') bits(7) ('@') 
     ; /seqhdr(0) 
     ; obu(0) 
    db 0x2A ; forbidden(1) ('*') obu_type(4) ('*') obu_extension_flag(1) ('*') obu_has_size_field(1) ('*') obu_reserved_1bit(1) ('*') 
    db 0x06 ; leb128_byte(8) 
    db 0x04 ; leb128_byte(8) 
    db 0xB5 ; itu_t_t35_country_code(8) 
    db 0x00, 0x3C ; itu_t_t35_terminal_provider_code(16) 
    db 0x00, 0x01 ; itu_t_t35_terminal_provider_oriented_code(16) 
     ; obu(0) 
    db 0x32 ; forbidden(1) ('2') obu_type(4) ('2') obu_extension_flag(1) ('2') obu_has_size_field(1) ('2') obu_reserved_1bit(1) ('2') 
    db 0x01 ; leb128_byte(8) 
    db 0x80 ; show_existing_frame(1) frame_to_show_map_idx(3) bits(4) 
mdat_end:

; vim: syntax=nasm
