%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1", "miaf"

ftyp_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"
dd BE(0)

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
dd BE(0)
dd BE(0)
db "pict"
dd BE(0)
dd BE(0)
dd BE(0)
hdlr_end:

pitm_start:
dd BE(pitm_end - pitm_start)
db "pitm"
dd BE(0)
db 0x00, 0x01
pitm_end:

iinf_start:
    dd BE(iinf_end - iinf_start)
    dd "iinf"
    db 0x00 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    db 0x00, 0x01 ; "entry_count(16)" 
    infe_start:
        dd BE(infe_end - infe_start)
        dd "infe"
        db 0x02 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x01 ; "item_ID(16)" 
        db 0x00, 0x00 ; "item_protection_index(16)" 
        db 0x61, 0x76, 0x30, 0x31 ; "item_type(32)" ('av01') 
        db 0x00 ; "item_name(8)" 
    infe_end:
iinf_end:

iprp_start:
dd BE(iprp_end - iprp_start)
db "iprp"
ipco_start:
dd BE(ipco_end - ipco_start)
db "ipco"
ispe_start:
dd BE(ispe_end - ispe_start)
db "ispe"
dd 0, 0, 0
ispe_end:
pixi_start:
    dd BE(pixi_end - pixi_start)
    dd "pixi"
pixi_end:
ipco_end:
ipma_start:
    dd BE(ipma_end - ipma_start)
    dd "ipma"
    db 0x00 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
    db 0x00, 0x01 ; "item_ID(16)" 
    db 0x02 ; "association_count(8)"  
    db 0x81 ; "essential(1)" "property_index(7)"
    db 0x82 ; "essential(1)" "property_index(7)"
ipma_end:
iprp_end:
meta_end:

moov_start:
dd BE(moov_end - moov_start)
db "moov"
mvhd_start:
dd BE(mvhd_end - mvhd_start)
db "mvhd"
mvhd_end:
trak_start:
dd BE(trak_end - trak_start)
db "trak"

mdia1_start:
dd BE(mdia1_end - mdia1_start)
db "mdia"
hdlr1_start:
dd BE(hdlr1_end - hdlr1_start)
db "hdlr"
dd BE(0)
dd BE(0)
dd "pict"
dd 0, 0, 0
hdlr1_end:
mdia1_end:

tkhd_start:
dd BE(tkhd_end - tkhd_start)
db "tkhd"
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)

dd BE(0x00010000), BE(0x00010000), 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, BE(0x40000000)

dd BE(0) ; width
dd BE(0) ; height

tkhd_end:
trak_end:

trak2_start:
dd BE(trak2_end - trak2_start)
db "trak"

mdia2_start:
dd BE(mdia2_end - mdia2_start)
db "mdia"
hdlr2_start:
dd BE(hdlr2_end - hdlr2_start)
db "hdlr"
dd BE(0)
dd BE(0)
dd "thmb"
dd 0, 0, 0
hdlr2_end:
mdia2_end:

tkhd2_start:
dd BE(tkhd2_end - tkhd2_start)
db "tkhd"
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)

dd BE(0x00010000), BE(0x00010000), 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, BE(0x40000000)

dd BE(0) ; width
dd BE(0) ; height

tkhd2_end:
trak2_end:
moov_end: