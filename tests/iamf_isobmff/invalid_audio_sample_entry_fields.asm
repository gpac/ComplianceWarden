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
    db 0,2 ; channelcount
    db 0,16 ; samplesize
    db 0,0 ; pre_defined
    db 0,0 ; reserved
    db 0,0,172,68 ; samplerate (44100)

    iacb_start:
        dd BE(iacb_end - iacb_start)
        dd "iacb"
        db 1 ; configurationVersion
        db 8 ; configOBUs_size
        ; We lazily add only an IA Sequence Header OBU here. This triggers an error
        ; for missing subsequent descriptor OBUs, but since we are not testing
        ; descriptor OBUs in this test, it is fine.
        ; IA Sequence Header OBU
        db 11111000b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant_copy = 0, trimming = 0, extension = 0
        db 6         ; obu_size = 6 bytes
        db 'iamf'    ; ia_code
        db 0         ; primary_profile
        db 0         ; additional_profile
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

stbl_end:
minf_end:
mdia_end:
trak_end:
moov_end:
