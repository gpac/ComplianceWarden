%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftip" ; error: mistyped 'ftyp'

db "isom"
dd BE(0x00)
db "isom", "iso2", "mp41"
ftyp_end:

; error: no 'moov' box
