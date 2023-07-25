%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
dd "ftyp"
db 0x68, 0x65, 0x69, 0x63 ; "major_brand" 
db 0x0, 0x0, 0x0, 0x0 ; "minor_version" 
db 0x6D, 0x69, 0x66, 0x31 ; "compatible_brand" 
db 0x68, 0x65, 0x69, 0x63 ; "compatible_brand" 
ftyp_end:

aaaa_start:
dd BE(aaaa_end - aaaa_start)
db "aaaa"
aaaa_end:
