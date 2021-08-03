%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"
db "isom"
dd BE(0x00)
db "mif1"
ftyp_end:

moov_start:
dd BE(moov_end - moov_start)
db "moov"
moov_end:
moov2_start:
dd BE(moov2_end - moov2_start)
db "moov"
moov2_end:
