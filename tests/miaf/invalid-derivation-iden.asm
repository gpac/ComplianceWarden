%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x61, 0x76, 0x69, 0x66 ; "brand(32)" ('avif') 
    db 0x00, 0x00, 0x00, 0x00 ; "version(32)" 
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
        db 0x70, 0x69, 0x63, 0x74 ; "handler_type(32)" ('pict') 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved1(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved2(32)" 
        db 0x00, 0x00, 0x00, 0x00 ; "reserved3(32)" 
        db 0x00 ; "name(8)" 
    hdlr_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x02 ; "item_ID(16)" 
    pitm_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x03 ; "entry_count(16)" 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x01 ; "flags(24)" 
            db 0x00, 0x01 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x69, 0x64, 0x65, 0x6E ; "item_type(32)" ('iden') 
            db 0x00 ; "item_name(8)" 
        infe_end:
        infe2_start:
            dd BE(infe2_end - infe2_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x02 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x69, 0x64, 0x65, 0x6E ; "item_type(32)" ('iden') 
            db 0x00 ; "item_name(8)" 
        infe2_end:
        infe3_start:
            dd BE(infe3_end - infe3_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x01 ; "flags(24)" 
            db 0x00, 0x03 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x69, 0x64, 0x65, 0x6E ; "item_type(32)" ('iden') 
            db 0x00 ; "item_name(8)" 
        infe3_end:
    iinf_end:
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x01 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x44 ; "offset_size(4)" ('D') "length_size(4)" ('D') 
        db 0x00 ; "base_offset_size(4)" "index_size(4)" 
        db 0x00, 0x03 ; "item_count(16)" 
        db 0x00, 0x01 ; "item_ID(16)" 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        db 0x00, 0x1B, 0x4A, 0x1D ; "extent_offset(32)" 
        db 0x00, 0x02, 0xB0, 0x85 ; "extent_length(32)" 
        db 0x00, 0x02 ; "item_ID(16)" 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        db 0x00, 0x00, 0x00, 0x00 ; "extent_offset(32)" 
        db 0x00, 0x00, 0x00, 0x08 ; "extent_length(32)" 
        db 0x00, 0x03 ; "item_ID(16)" 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        db 0x00, 0x00, 0x05, 0x3C ; "extent_offset(32)" 
        db 0x00, 0x02, 0x0C, 0xD4 ; "extent_length(32)" 
    iloc_end:
    iref_start:
        dd BE(iref_end - iref_start)
        dd "iref"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x00, 0x00, 0x08 ; "box_size(32)" 
        db 0x64, 0x69, 0x6D, 0x67 ; "box_type(32)" ('dimg') 
        db 0x00, 0x02 ; "from_item_ID(16)" 
        db 0x00, 0x01 ; "reference_count(16)" 
        db 0x00, 0x01 ; "to_item_ID(16)" 
    iref_end:
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
