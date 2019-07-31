%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))
%define fourcc(a) db a

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
fourcc("moov")
trak_start:
dd BE(trak_end - trak_start)
fourcc("trak")
mdia_start:
dd BE(mdia_end - mdia_start)
fourcc("mdia")
minf_start:
dd BE(minf_end - minf_start)
fourcc("minf")
stbl_start:
dd BE(stbl_end - stbl_start)
fourcc("stbl")
stsd_start:
dd BE(stsd_end - stsd_start)
fourcc("stsd")
dd BE(0)
dd BE(1) ; entry_count

avc1_start:
dd BE(avc1_end - avc1_start)
fourcc("avc1")
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0) ; width, height
dd BE(0) ; horizresolution
dd BE(0) ; vertresolution
dd BE(0)
db 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
dd BE(0)
clli_start:
dd BE(clli_end - clli_start)
fourcc("clli")
dd BE(0)
clli_end:
mdcv_start:
dd BE(mdcv_end - mdcv_start)
fourcc("mdcv")
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
dd BE(0)
mdcv_end:
avc1_end:

stsd_end:
stbl_end:
minf_end:
mdia_end:
trak_end:
moov_end:

; vim: syntax=nasm
