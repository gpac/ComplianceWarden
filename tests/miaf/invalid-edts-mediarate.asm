%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

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
dd BE(0)
dd BE(0)
dd "pict"
dd 0, 0, 0
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
dd 0
dd BE(2)
dd BE(0) ; edit_duration
dd BE(0) ; media_time
db 0, 0  ; media_rate
dw BE(0)
dd BE(0) ; edit_duration
dd BE(0) ; media_time
db 0, 2  ; media_rate
dw BE(0)

elst_end:
edts_end:
trak_end:
moov_end:
