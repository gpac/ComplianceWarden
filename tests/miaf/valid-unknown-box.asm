%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1", "miaf"

ftyp_end:

;"Other boxes may be present in the file but they shall not affect the processing"
xxxx1_start:
dd BE(xxxx1_end - xxxx1_start)
db "xxxx"
xxxx1_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
hdlr_end:

;"Other boxes may be present in the file but they shall not affect the processing"
xxxx2_start:
dd BE(xxxx2_end - xxxx2_start)
db "xxxx"
xxxx2_end:

meta_end:

;"Other boxes may be present in the file but they shall not affect the processing"
xxxx3_start:
dd BE(xxxx3_end - xxxx3_start)
db "xxxx"
xxxx3_end:

; vim: syntax=nasm
