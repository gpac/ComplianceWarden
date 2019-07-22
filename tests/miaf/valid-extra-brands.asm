%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1", "miaf", "dumy", "dumy"

ftyp_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
hdlr_end:

meta_end:

; vim: syntax=nasm
