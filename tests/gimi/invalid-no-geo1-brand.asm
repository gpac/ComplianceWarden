%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x61, 0x76, 0x69, 0x73 ; "major_brand(32)" ('avis') 
    db 0x00, 0x00, 0x00, 0x00 ; "minor_version(32)" 
    db 0x6D, 0x69, 0x66, 0x31 ; "compatible_brand(32)" ('mif1') 
    db 0x61, 0x76, 0x69, 0x66 ; "compatible_brand(32)" ('avif') 
    db 0x61, 0x76, 0x30, 0x31 ; "compatible_brand(32)" ('av01') 
    db 0x61, 0x76, 0x69, 0x73 ; "compatible_brand(32)" ('avis') 
    db 0x6D, 0x73, 0x66, 0x31 ; "compatible_brand(32)" ('msf1')  
    db 0x69, 0x73, 0x6F, 0x37 ; "compatible_brand(32)" ('iso7')
    db 0x6D, 0x69, 0x61, 0x66 ; "compatible_brand(32)" ('miaf') 
    db 0x4D, 0x41, 0x31, 0x42 ; "compatible_brand(32)" ('MA1B') 
ftyp_end: