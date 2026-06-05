%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))
%define fourcc(a) db a

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x69, 0x73, 0x6F, 0x6D ; major_brand(32) ('isom')
    db 0x00, 0x00, 0x02, 0x00 ; minor_version(32)
    db 0x69, 0x73, 0x6F, 0x6D ; compatible_brand(32) ('isom')
    db 0x69, 0x61, 0x6D, 0x66 ; compatible_brand(32) ('iamf')
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
    db 0x00, 0x00, 0x00, 0x00 ; creation_time(32)
    db 0x00, 0x00, 0x00, 0x00 ; modification_time(32)
    db 0x00, 0x00, 0x00, 0x01 ; track_ID(32)
    db 0x00, 0x00, 0x00, 0x00 ; reserved(32)
    db 0x00, 0x00, 0x00, 0x01 ; duration(32)
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
    db 0x00, 0x00, 0x00, 0x00 ; width(32)
    db 0x00, 0x00, 0x00, 0x00 ; height(32)
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
db 0x73, 0x6F, 0x75, 0x6E ; handler_type(32) ('soun')
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
smhd_start:
    dd BE(smhd_end - smhd_start)
    dd "smhd"
    db 0x00 ; version(8)
    db 0x00, 0x00, 0x00 ; flags(24)
    db 0x00, 0x00 ; balance(16)
    db 0x00, 0x00 ; reserved(16)
smhd_end:

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

iamf_start:
    dd BE(iamf_end - iamf_start)
    dd "iamf"
    ; SampleEntry fields (8 bytes)
    db 0,0,0,0,0,0 ; reserved
    db 0,1 ; data_reference_index

    ; AudioSampleEntry fields (20 bytes)
    db 0,0 ; version
    db 0,0,0,0,0,0 ; reserved
    db 0,0 ; channelcount
    db 0,16 ; samplesize
    db 0,0 ; pre_defined
    db 0,0 ; reserved
    db 0,0,0,0 ; samplerate


    iacb_start:
        dd BE(iacb_end - iacb_start)
        dd "iacb"
        db 1 ; configurationVersion
        db iacb_end - iacb_start - 10 ; configOBUs_size
        ; IA Sequence Header OBU
        db 11111000b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant_copy = 0, trimming = 0, extension = 0
        db 6         ; obu_size = 6 bytes
        db 'iamf'    ; ia_code
        db 0         ; primary_profile
        db 0         ; additional_profile

        ; Codec Config OBU (Valid ipcm)
        db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
        db 14        ; obu_size = 14 bytes
        db 0         ; codec_config_id = 0
        db 'ipcm'    ; codec_id = 'ipcm'
        db 100       ; num_samples_per_frame = 100
        db 0         ; audio_roll_distance (high byte)
        db 0         ; audio_roll_distance (low byte)
        db 1         ; sample_format_flags = 1 (little-endian)
        db 16        ; sample_size = 16
        db 0, 0, 0xAC, 0x44 ; sample_rate = 44100 (big-endian)

        ; Audio Element OBU (Valid)
        db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
        db 10        ; obu_size = 10 bytes
        db 1         ; audio_element_id = 1
        db 0         ; audio_element_type = 0 (CHANNEL_BASED)
        db 0         ; codec_config_id = 0
        db 1         ; num_substreams = 1
        db 1         ; audio_substream_id = 1
        db 0         ; num_parameters = 0
        db 00100000b ; num_layers = 1
        db 0         ; loudspeaker_layout = 0 (Mono)
        db 1         ; substream_count = 1
        db 0         ; coupled_substream_count = 0

        ; Mix Presentation OBU (Valid)
        mix_obu_start:
        db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
        db mix_obu_end - mix_obu_start - 2 ; obu_size
        db 1         ; mix_presentation_id = 1
        db 0         ; count_label = 0
        db 1         ; num_sub_mixes = 1
        ; Sub-mix 1
        db 1         ; num_audio_elements = 1
        db 1         ; audio_element_id = 1
        db 0         ; headphones_rendering_mode = 0
        db 0         ; rendering_config_extension_size = 0
        ; element_mix_gain
        db 0         ; parameter_id = 0
        db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
        db 10000000b ; param_definition_mode = 1
        db 0, 0      ; default_mix_gain = 0
        ; output_mix_gain
        db 1         ; parameter_id = 1
        db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
        db 10000000b ; param_definition_mode = 1
        db 0, 0      ; default_mix_gain = 0
        db 1         ; num_layouts = 1
        ; Layout 1
        db 10000000b ; layout_type = 2, sound_system = 0 (Stereo)
        ; Loudness
        db 0         ; info_type = 0
        db 0, 0      ; integrated_loudness = 0
        db 0, 0      ; digital_peak = 0
        mix_obu_end:
    iacb_end:

iamf_end:

stsd_end:

stts_start:
    dd BE(stts_end - stts_start)
    dd "stts"
    db 0x00 ; version(8)
    db 0x00, 0x00, 0x00 ; flags(24)
    db 0x00, 0x00, 0x00, 0x00 ; entry_count(32)
stts_end:

stsc_start:
    dd BE(stsc_end - stsc_start)
    dd "stsc"
    db 0x00 ; version(8)
    db 0x00, 0x00, 0x00 ; flags(24)
    db 0x00, 0x00, 0x00, 0x00 ; entry_count(32)
stsc_end:

stsz_start:
    dd BE(stsz_end - stsz_start)
    dd "stsz"
    db 0x00 ; version(8)
    db 0x00, 0x00, 0x00 ; flags(24)
    db 0x00, 0x00, 0x00, 0x00 ; sample_size(32)
    db 0x00, 0x00, 0x00, 0x00 ; sample_count(32)
stsz_end:

stco_start:
    dd BE(stco_end - stco_start)
    dd "stco"
    db 0x00 ; version(8)
    db 0x00, 0x00, 0x00 ; flags(24)
    db 0x00, 0x00, 0x00, 0x00 ; entry_count(32)
stco_end:

stss_start:
    dd BE(stss_end - stss_start)
    dd "stss"
    db 0x00 ; version(8)
    db 0x00, 0x00, 0x00 ; flags(24)
    db 0x00, 0x00, 0x00, 0x00 ; entry_count(32)
stss_end:

stbl_end:
minf_end:
mdia_end:
trak_end:
moov_end:
