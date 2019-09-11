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
dd BE(1) ; 1 item
dd BE(0) ; construction_method(1)
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
dd BE(0xaabb) ; from_item_ID
db 0x00, 0x01 ; reference_count
dd BE(0)      ; to_item_ID
thmb_end:
auxl_start:
dd BE(auxl_end - auxl_start)
dd "auxl"
dd BE(0xaabb) ; from_item_ID
db 0x00, 0x01 ; reference_count
dd BE(0)      ; to_item_ID
auxl_end:
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
dd 0, 0, 0
ispe_end:
ipco_end:
iprp_end:

meta_end:

; vim: syntax=nasm
