%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

meta_start:
dd BE(meta_end - meta_start)
db "meta"

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
hdlr_end:

dinf_start:
dd BE(dinf_end - dinf_start)
db "dinf"
dinf_end:

meta_end:

;"Other boxes may be present in the file but they shall not affect the processing"
xxxx_start:
dd BE(xxxx_end - xxxx_start)
db "xxxx"
xxxx_end:

; vim: syntax=nasm
