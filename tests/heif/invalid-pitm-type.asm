%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x6D, 0x69, 0x66, 0x31 ; brand(32) ('mif1') 
    db 0x00, 0x00, 0x00, 0x00 ; version(32) 
    db 0x6D, 0x69, 0x66, 0x31 ; compatible_brand(32) ('mif1') 
ftyp_end:
meta_start:
    dd BE(meta_end - meta_start)
    dd "meta"
    db 0x00 ; version(8) 
    db 0x00, 0x00, 0x00 ; flags(24) 
    hdlr_start:
        dd BE(hdlr_end - hdlr_start)
        dd "hdlr"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
        db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
        db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
        db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
        db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
        db 0x70 ; name(8) ('p') 
        db 0x69 ; name(8) ('i') 
        db 0x63 ; name(8) ('c') 
        db 0x74 ; name(8) ('t') 
        db 0x00 ; name(8) 
    hdlr_end:
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x01 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x44 ; offset_size(4) ('D') length_size(4) ('D') 
        db 0x40 ; base_offset_size(4) ('@') index_size(4) ('@') 
        db 0x00, 0x01 ; item_count(16) 
        db 0x00, 0x01 ; item_ID(16) 
        db 0x00, 0x01 ; reserved2(12) construction_method(4) 
        db 0x00, 0x00 ; data_reference_index(16) 
        db 0x00, 0x00, 0x00, 0x00 ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
        db 0x00, 0x00, 0x00, 0x00 ; extent_offset(32) 
        db 0x00, 0x00, 0x00, 0x64 ; extent_length(32) 
    iloc_end:
    idat_start:
        dd BE(idat_end - idat_start)
        dd "idat"
    idat_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x01 ; entry_count(16) 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x00, 0x00 ; item_protection_index(16) 
            db 0x61, 0x61, 0x61, 0x61 ; item_type(32) ('aaaa') 
            db 0x69 ; item_name(8) ('i') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
            db 0x00 ; item_name(8) 
        infe_end:
    iinf_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x01 ; item_ID(16) 
    pitm_end:
    iprp_start:
        dd BE(iprp_end - iprp_start)
        dd "iprp"
        ipco_start:
            dd BE(ipco_end - ipco_start)
            dd "ipco"
            avcC_start:
                dd BE(avcC_end - avcC_start)
                dd "avcC"
                db 0x00 ; configurationVersion(8) 
                db 0x42 ; AVCProfileIndication(8) ('B') 
                db 0x00 ; profile_compatibility(8) 
                db 0x00 ; AVCLevelIndication(8) 
                db 0x00 ; reserved7(6) lengthSizeMinusOne(2) 
                db 0x00 ; reserved8(3) numOfSequenceParameterSets(5) 
                db 0x00 ; numOfPictureParameterSets(8) 
            avcC_end:
            ispe_start:
                dd BE(ispe_end - ispe_start)
                dd "ispe"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x00, 0x00, 0x01, 0x40 ; image_width(32) 
                db 0x00, 0x00, 0x00, 0xF0 ; image_height(32) 
            ispe_end:
            pixi_start:
                dd BE(pixi_end - pixi_start)
                dd "pixi"
            pixi_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x00 ; flags(24) 
            db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
            db 0x00, 0x01 ; item_ID(16) 
            db 0x03 ; association_count(8) 
            db 0x81 ; essential(1) property_index(7) 
            db 0x02 ; essential(1) property_index(7) 
            db 0x03 ; essential(1) property_index(7) 
        ipma_end:
    iprp_end:
meta_end:

; vim: syntax=nasm
