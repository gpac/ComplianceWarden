%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1", "miaf"

ftyp_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"
dd BE(0)

hdlr_start:
dd BE(hdlr_end - hdlr_start)
db "hdlr"
dd BE(0)
dd BE(0)
db "pict"
dd BE(0)
dd BE(0)
dd BE(0)
hdlr_end:

pitm_start:
dd BE(pitm_end - pitm_start)
db "pitm"
dd BE(0)
db 0xaa, 0xbb
pitm_end:

iloc_start:
dd BE(iloc_end - iloc_start)
db "iloc"
dd BE(0x01000000)
dd BE(2) ; 2 items
dd BE(0x00030000) ; construction_method(1)
dw 0
dw 0
dd BE(0x00040000) ; construction_method(1)
dw 0
dw 0
iloc_end:

iref_start:
dd BE(iref_end - iref_start)
db "iref"
db 0x01, 0x00, 0x00, 0x00 ; version=1
thmb_start:
dd BE(thmb_end - thmb_start)
dd "thmb"
dd BE(3)      ; from_item_ID
db 0x00, 0x01 ; reference_count
dd BE(3)      ; to_item_ID
thmb_end:
iref_end:

iinf_start:
dd BE(iinf_end - iinf_start)
db "iinf"
dd BE(0)
db 0x00, 0x00
iinf_end:

iprp_start:
dd BE(iprp_end - iprp_start)
db "iprp"
ipco_start:
dd BE(ipco_end - ipco_start)
db "ipco"
ispe_start:
dd BE(ispe_end - ispe_start)
db "ispe"
dd 0
dd BE(1), BE(1) ; width, height
ispe_end:
ispe2_start:
dd BE(ispe2_end - ispe2_start)
db "ispe"
dd 0
dd BE(100), BE(100) ; width, height
ispe2_end:
ipco_end:
ipma_start:
    dd BE(ipma_end - ipma_start)
    dd "ipma"
    db 0x00 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    db 0x00, 0x00, 0x00, 0x02 ; "entry_count(32)" 
    db 0x00, 0x03 ; "item_ID(16)" 
    db 0x01 ; "association_count(8)" 
    db 0x82 ; "essential(1)" "property_index(7)" 
    db 0x00, 0x04 ; "item_ID(16)" 
    db 0x01 ; "association_count(8)" 
    db 0x81 ; "essential(1)" "property_index(7)"
ipma_end:
iprp_end:

meta_end:

; vim: syntax=nasm
