%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x6A, 0x32, 0x6B, 0x69 ; major_brand(32) ('j2ki')
    db 0x00, 0x00, 0x00, 0x00 ; minor_version(32)
    db 0x6D, 0x69, 0x66, 0x31 ; compatible_brand(32) ('mif1')
    db 0x6A, 0x32, 0x6B, 0x69 ; compatible_brand(32) ('j2ki')
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
        db 0x00 ; name(8)
    hdlr_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x00, 0x01 ; item_ID(16) 
    pitm_end:
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
            db 0x6A, 0x32, 0x6B, 0x31 ; item_type(32) ('j2k1')
            db 0x00 ; item_name(8)
        infe_end:
    iinf_end:
    iprp_start:
        dd BE(iprp_end - iprp_start)
        dd "iprp"
        ipco_start:
            dd BE(ipco_end - ipco_start)
            dd "ipco"
            ispe_start:
                dd BE(ispe_end - ispe_start)
                dd "ispe"
                db 0x00 ; version(8)
                db 0x00, 0x00, 0x00 ; flags(24)
                db 0x00, 0x00, 0x06, 0x2E ; image_width(32)
                db 0x00, 0x00, 0x07, 0x46 ; image_height(32)
            ispe_end:
            colr_start:
                dd BE(colr_end - colr_start)
                dd "colr"
                dd "nclx"
                db 0x00
                db 0x01
                db 0x00
                db 0x0D
                db 0x00
                db 0x06
                db 0x80
            colr_end:
            j2kH_start:
                dd BE(j2kH_end - j2kH_start)
                dd "j2kH"
                cdef_start:
                    dd BE(cdef_end - cdef_start)
                    dd "cdef"
                    db 0x00, 0x03 ; channel_count(16)
                    db 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 ; channel_0
                    db 0x00, 0x01, 0x00, 0x00, 0x00, 0x02 ; channel_1
                    db 0x00, 0x02, 0x00, 0x00, 0x00, 0x03 ; channel_2
                cdef_end:
            j2kH_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; version(8)
            db 0x00, 0x00, 0x00 ; flags(24)
            db 0x00, 0x00, 0x00, 0x01 ; entry_count(32)
            db 0x00, 0x01 ; item_ID(16)
            db 0x03 ; association_count(8)
            db 0x01 ; essential(1) property_index(7)
            db 0x02 ; essential(1) property_index(7)
            db 0x83 ; essential(1) property_index(7)
        ipma_end:
    iprp_end:
meta_end:
