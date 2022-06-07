%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x69, 0x73, 0x6F, 0x6D ; major_brand(32) ('isom') 
    db 0x00, 0x00, 0x02, 0x00 ; minor_version(32) 
    db 0x69, 0x73, 0x6F, 0x6D ; compatible_brand(32) ('isom') 
    db 0x61, 0x76, 0x30, 0x31 ; compatible_brand(32) ('av01') 
    db 0x69, 0x73, 0x6F, 0x32 ; compatible_brand(32) ('iso2') 
    db 0x6D, 0x70, 0x34, 0x31 ; compatible_brand(32) ('mp41') 
ftyp_end:
free_start:
    dd BE(free_end - free_start)
    dd "free"
free_end:
mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
mdat_end:

; vim: syntax=nasm
