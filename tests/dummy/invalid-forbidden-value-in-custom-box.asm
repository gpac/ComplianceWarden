%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1"
ftyp_end:

moov_start:
dd BE(moov_end - moov_start)
db "moov"

moov_end:

dumy_start:
dd BE(dumy_end - dumy_start)
db "dumy"
dd BE(666)
dumy_end:

; vim: syntax=nasm
