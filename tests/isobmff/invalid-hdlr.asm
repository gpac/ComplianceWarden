%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x61, 0x76, 0x69, 0x66 ; brand(32) ('avif') 
    db 0x00, 0x00, 0x00, 0x00 ; version(32) 
    db 0x61, 0x76, 0x69, 0x66 ; compatible_brand(32) ('avif') 
ftyp_end:
meta_start:
    dd 0
    dd "meta"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    
    hdlr_start:
        dd BE(hdlr_end - hdlr_start)
        dd "hdlr"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x01 ; pre_defined(32) 
        db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
        db 0x00, 0x00, 0x00, 0x01 ; reserved1(32) 
        db 0x00, 0x00, 0x00, 0x01 ; reserved2(32) 
        db 0x00, 0x00, 0x00, 0x01 ; reserved3(32) 
        db 0x01 ; name(8) 
    hdlr_end:
meta_end:

; vim: syntax=nasm
