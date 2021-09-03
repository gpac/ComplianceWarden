%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x00)
db "mif1", "miaf"

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
db "pitm"
dd BE(0)
db 0xaa, 0xbb
pitm_end:

iloc_start:
dd BE(iloc_end - iloc_start)
dd "iloc"
db 0x00 ; version(8) 
db 0x00, 0x00, 0x00 ; flags(24) 
db 0x04 ; offset_size(4) length_size(4) 
db 0x40 ; base_offset_size(4) ('@') reserved1(4) ('@') 
db 0x00, 0x02 ; item_count(16) 
db 0x00, 0x01 ; item_ID(16) 
db 0x00, 0x00 ; data_reference_index(16) 
dd BE(mdat_start - ftyp_start + 8) ; base_offset(32) 
db 0x00, 0x01 ; extent_count(16) 
; extent_offset(0) 
db 0x00, 0x00, 0x00, 0x01 ; extent_length(32) 
db 0x00, 0x02 ; item_ID(16) 
db 0x00, 0x00 ; data_reference_index(16) 
dd BE(mdat_start - ftyp_start + 8) ; base_offset(32) 
db 0x00, 0x01 ; extent_count(16) 
; extent_offset(0) 
db 0x00, 0x00, 0x00, 0x08 ; extent_length(32) 
iloc_end:

iref_start:
dd BE(iref_end - iref_start)
db "iref"
db 0x01, 0x00, 0x00, 0x00 ; version=1
db 0x00, 0x00, 0x00, 0x0E ; "box_size(32)" 
db "dimg" ; "box_type(32)" 
dd BE(2) ; "from_item_ID(32)" 
db 0x00, 0x01 ; "reference_count(16)" 
dd BE(1) ; "to_item_ID(32)" 
iref_end:

iinf_start:
dd BE(iinf_end - iinf_start)
db "iinf"
dd BE(0)
db 0, 2 ; 2 entries
infe_start:
    dd BE(infe_end - infe_start)
    dd "infe"
    db 0x02 ; "version(8)" 
    db 0x00, 0x00, 0x01 ; "flags(24)" 
    db 0x00, 0x01 ; "item_ID(16)" 
    db 0x00, 0x00 ; "item_protection_index(16)" 
    db 0x61, 0x76, 0x30, 0x31 ; "item_type(32)" 
    db 0x00 ; "item_name(8)" 
infe_end:
infe2_start:
    dd BE(infe2_end - infe2_start)
    dd "infe"
    db 0x02 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    db 0x00, 0x02 ; "item_ID(16)" 
    db 0x00, 0x00 ; "item_protection_index(16)" 
    db 0x67, 0x72, 0x69, 0x64 ; "item_type(32)" 
    db 0x00 ; "item_name(8)" 
infe2_end:
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
dd BE(10), BE(10) ; width, height
ispe2_end:
pixi_start:
dd BE(pixi_end - pixi_start)
db "pixi"
pixi_end:
ipco_end:
ipma_start:
    dd BE(ipma_end - ipma_start)
    dd "ipma"
    db 0x00 ; "version(8)" 
    db 0x00, 0x00, 0x00 ; "flags(24)" 
    db 0x00, 0x00, 0x00, 0x02 ; "entry_count(32)" 
    db 0x00, 0x01 ; "item_ID(16)" 
    db 0x01 ; "association_count(8)" 
    db 0x82 ; "essential(1)" "property_index(7)" 
    db 0x00, 0x02 ; "item_ID(16)" 
    db 0x02 ; "association_count(8)" 
    db 0x82 ; "essential(1)" "property_index(7)" 
    db 0x83 ; "essential(1)" "property_index(7)"
ipma_end:
iprp_end:

meta_end:

mdat_start:
dd BE(mdat_end - mdat_start)
db "mdat"
db 0x00 ; derivation version
db 0x00 ; dimg flags
db 0x00 ; dimg rows_minus_one
db 0x00 ; dimg columns_minus_one
db 0x00, 0x00 ; dimg output_width
db 0x00, 0x00 ; dimg output_height
mdat_end:

; vim: syntax=nasm
