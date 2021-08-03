%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x6D, 0x69, 0x66, 0x31 ; "major_brand(32)" 
    db 0x00, 0x00, 0x00, 0x00 ; "minor_version(32)" 
    db 0x6D, 0x69, 0x66, 0x31 ; "compatible_brand(32)" ('mif1') 
    db 0x6D, 0x69, 0x61, 0x66 ; "compatible_brand(32)" ('miaf') 
ftyp_end:
meta_start:
    dd BE(meta_end - meta_start)
    dd "meta"
    db 0x00 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    hdlr_start:
        dd BE(hdlr_end - hdlr_start)
        dd "hdlr"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x00, 0x00, 0x00 ; "pre_defined(32)" 
        db 0x70, 0x69, 0x63, 0x74 ; "handler_type(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved1(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved2(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved3(32)" 
        db 0x00 ; "name(8)" 
    hdlr_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x01 ; "entry_count(16)" 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x01 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x61, 0x76, 0x63, 0x31 ; "item_type(32)" 
            db 0x00 ; "item_name(8)" 
        infe_end:
    iinf_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x01 ; "item_ID(16)" 
    pitm_end:
    iprp_start:
        dd BE(iprp_end - iprp_start)
        dd "iprp"
        ipco_start:
            dd BE(ipco_end - ipco_start)
            dd "ipco"
            ispe_start:
                dd BE(ispe_end - ispe_start)
                dd "ispe"
                db 0x00 ; "version(8)" 
                db 0x00, 0x00, 0x00 ; "flags(24)" 
                db 0x00, 0x00, 0x00, 0x00 ; "image_width(32)" 
                db 0x00, 0x00, 0x00, 0x00 ; "image_height(32)" 
            ispe_end:
        ipco_end:
    iprp_end:
meta_end:
; vim: syntax=nasm
