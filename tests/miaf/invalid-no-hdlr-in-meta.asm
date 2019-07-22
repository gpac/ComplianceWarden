%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

meta_start:
dd BE(meta_end - meta_start)
db "meta"

dinf_start:
dd BE(dinf_end - dinf_start)
db "dinf"
dinf_end:

meta_end:

; vim: syntax=nasm
