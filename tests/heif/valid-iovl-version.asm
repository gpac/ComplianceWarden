%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x00)
db "mif1"

ftyp_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"
dd BE(0)

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
db 0x00 ; version(8) 
db 0x00, 0x00, 0x00 ; flags(24) 
db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
db 0x00 ; name(8) 
hdlr_end:

pitm_start:
dd BE(pitm_end - pitm_start)
dd "pitm"
db 0x00 ; "version(8)" 
db 0x00, 0x00, 0x00 ; "flags(24)" 
db 0x00, 0x0A ; "item_ID(16)" 
pitm_end:

iinf_start:
dd BE(iinf_end - iinf_start)
db "iinf"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x01 ; entry_count(16) 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x0A ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x69, 0x6F, 0x76, 0x6C ; item_type(32) ('iovl') 
            db 0x00 ; item_name(8) 
        infe_end:
iinf_end:

iloc_start:
    dd BE(iloc_end - iloc_start)
    dd "iloc"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    db 0x04 ; offset_size(4) length_size(4) 
    db 0x40 ; base_offset_size(4) ('@') reserved1(4) ('@') 
    db 0x00, 0x01 ; item_count(16) 
    db 0x00, 0x0A ; item_ID(16) 
    db 0x00, 0x00 ; data_reference_index(16) 
    dd BE(mdat_start - ftyp_start + 8) ; base_offset(32) 
    db 0x00, 0x01 ; extent_count(16) 
    ; extent_offset(0) 
    db 0x00, 0x00, 0x00, 0x01 ; extent_length(32) 
iloc_end:

iprp_start:
dd BE(iprp_end - iprp_start)
db "iprp"
ipco_start:
dd BE(ipco_end - ipco_start)
db "ipco"
ispe_start:
dd BE(ispe_end - ispe_start)
db "ispe"
dd 0, 0, 0
ispe_end:
ipco_end:
iprp_end:

meta_end:

mdat_start:
dd BE(mdat_end - mdat_start)
db "mdat"
db 0x00 ; 'iovl' version
mdat_end:

; vim: syntax=nasm
