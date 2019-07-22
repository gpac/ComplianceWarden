%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1", "miaf"

ftyp_end:

;"FreeSpaceBox as defined in ISO/IEC 14496-12 may be present as permitted by that specification, including at top level"
free1_start:
dd BE(free1_end - free1_start)
db "free"
free1_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
hdlr_end:

;"FreeSpaceBox as defined in ISO/IEC 14496-12 may be present as permitted by that specification, including at top level"
free2_start:
dd BE(free2_end - free2_start)
db "free"
free2_end:

meta_end:

;"FreeSpaceBox as defined in ISO/IEC 14496-12 may be present as permitted by that specification, including at top level"
free3_start:
dd BE(free3_end - free3_start)
db "free"
free3_end:

; vim: syntax=nasm
