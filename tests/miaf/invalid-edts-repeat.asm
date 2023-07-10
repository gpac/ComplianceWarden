%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x00)
db "mif1", "miaf", "isom"

ftyp_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"
dd BE(0)

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

pitm_start:
dd BE(pitm_end - pitm_start)
db "pitm"
dd BE(0)
db 0x00, 0x00
pitm_end:

iinf_start:
dd BE(iinf_end - iinf_start)
db "iinf"
dd BE(0)
db 0x00, 0x00
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
ipco_end:
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

mdia_start:
dd BE(mdia_end - mdia_start)
db "mdia"
hdlr2_start:
dd BE(hdlr2_end - hdlr2_start)
db "hdlr"
db 0x00 ; version(8) 
db 0x00, 0x00, 0x00 ; flags(24) 
db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
dd "thmb" ; handler_type(32) ('thmb') 
db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
db 0x00 ; name(8) 
hdlr2_end:

minf_start:
dd BE(minf_end - minf_start)
dd "minf"
stbl_start:
dd BE(stbl_end - stbl_start)
dd "stbl"
stss_start:
dd BE(stss_end - stss_start)
dd "stss"
dd BE(0)
dd BE(4)
dd BE(0), BE(2), BE(6), BE(9)
stss_end:
stbl_end:
minf_end:

mdia_end:

edts_start:
dd BE(edts_end - edts_start)
db "edts"
elst_start:
dd BE(elst_end - elst_start)
db "elst"
dd 0     ; don't loop
dd BE(1)
dd BE(0) ; edit_duration
dd BE(0) ; media_time
db 0, 1  ; media_rate
dw BE(0)

elst_end:
edts_end:
trak_end:


trak2_start:
dd BE(trak2_end - trak2_start)
db "trak"

mdia2_start:
dd BE(mdia2_end - mdia2_start)
db "mdia"
hdlr22_start:
dd BE(hdlr22_end - hdlr22_start)
db "hdlr"
db 0x00 ; version(8) 
db 0x00, 0x00, 0x00 ; flags(24) 
db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
db 0x00 ; name(8) 
hdlr22_end:

minf2_start:
dd BE(minf2_end - minf2_start)
dd "minf"
stbl2_start:
dd BE(stbl2_end - stbl2_start)
dd "stbl"
stss2_start:
dd BE(stss2_end - stss2_start)
dd "stss"
dd BE(0)
dd BE(4)
dd BE(0), BE(2), BE(6), BE(9)
stss2_end:
stbl2_end:
minf2_end:

mdia2_end:

edts2_start:
dd BE(edts2_end - edts2_start)
db "edts"
elst2_start:
dd BE(elst2_end - elst2_start)
db "elst"
dd BE(1) ; loop
dd BE(1)
dd BE(0) ; edit_duration
dd BE(0) ; media_time
db 0, 1  ; media_rate
dw BE(0)

elst2_end:
edts2_end:
trak2_end:

moov_end:
