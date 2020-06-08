%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x61, 0x76, 0x69, 0x73 ; "brand(32)" ('avis') 
    db 0x00, 0x00, 0x00, 0x00 ; "version(32)" 
    db 0x6D, 0x69, 0x66, 0x31 ; "compatible_brand(32)" ('mif1') 
    db 0x61, 0x76, 0x69, 0x66 ; "compatible_brand(32)" ('avif') 
    db 0x69, 0x73, 0x6F, 0x34 ; "compatible_brand(32)" ('iso4') 
    db 0x61, 0x76, 0x30, 0x31 ; "compatible_brand(32)" ('av01') 
    db 0x61, 0x76, 0x69, 0x73 ; "compatible_brand(32)" ('avis') 
    db 0x6D, 0x73, 0x66, 0x31 ; "compatible_brand(32)" ('msf1') 
    db 0x6D, 0x69, 0x61, 0x66 ; "compatible_brand(32)" ('miaf') 
    db 0x4D, 0x41, 0x31, 0x42 ; "compatible_brand(32)" ('MA1B') 
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
        db 0x47 ; "name(8)" ('G') 
        db 0x50 ; "name(8)" ('P') 
        db 0x41 ; "name(8)" ('A') 
        db 0x43 ; "name(8)" ('C') 
        db 0x20 ; "name(8)" (' ') 
        db 0x70 ; "name(8)" ('p') 
        db 0x69 ; "name(8)" ('i') 
        db 0x63 ; "name(8)" ('c') 
        db 0x74 ; "name(8)" ('t') 
        db 0x20 ; "name(8)" (' ') 
        db 0x48 ; "name(8)" ('H') 
        db 0x61 ; "name(8)" ('a') 
        db 0x6E ; "name(8)" ('n') 
        db 0x64 ; "name(8)" ('d') 
        db 0x6C ; "name(8)" ('l') 
        db 0x65 ; "name(8)" ('e') 
        db 0x72 ; "name(8)" ('r') 
        db 0x00 ; "name(8)" 
    hdlr_end:
    pitm_start:
        dd BE(pitm_end - pitm_start)
        dd "pitm"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x04 ; "item_ID(16)" 
    pitm_end:
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x04 ; "offset_size(4)" "length_size(4)" 
        db 0x40 ; "base_offset_size(4)" ('@') "reserved1(4)" ('@') 
        db 0x00, 0x02 ; "item_count(16)" 
        db 0x00, 0x03 ; "item_ID(16)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x00, 0x08, 0xBA ; "base_offset(32)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        db 0x00, 0x00, 0x00, 0x42 ; "extent_length(32)" 
        db 0x00, 0x04 ; "item_ID(16)" 
        db 0x00, 0x00 ; "data_reference_index(16)" 
        db 0x00, 0x00, 0x08, 0xFC ; "base_offset(32)" 
        db 0x00, 0x01 ; "extent_count(16)" 
        db 0x00, 0x00, 0x00, 0xF5 ; "extent_length(32)" 
    iloc_end:
    iinf_start:
        dd BE(iinf_end - iinf_start)
        dd "iinf"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x02 ; "entry_count(16)" 
        infe_start:
            dd BE(infe_end - infe_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x03 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x61, 0x76, 0x30, 0x31 ; "item_type(32)" ('av01') 
            db 0x41 ; "item_name(8)" ('A') 
            db 0x6C ; "item_name(8)" ('l') 
            db 0x70 ; "item_name(8)" ('p') 
            db 0x68 ; "item_name(8)" ('h') 
            db 0x61 ; "item_name(8)" ('a') 
            db 0x00 ; "item_name(8)" 
        infe_end:
        infe2_start:
            dd BE(infe2_end - infe2_start)
            dd "infe"
            db 0x02 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x04 ; "item_ID(16)" 
            db 0x00, 0x00 ; "item_protection_index(16)" 
            db 0x61, 0x76, 0x30, 0x31 ; "item_type(32)" ('av01') 
            db 0x43 ; "item_name(8)" ('C') 
            db 0x6F ; "item_name(8)" ('o') 
            db 0x6C ; "item_name(8)" ('l') 
            db 0x6F ; "item_name(8)" ('o') 
            db 0x72 ; "item_name(8)" ('r') 
            db 0x00 ; "item_name(8)" 
        infe2_end:
    iinf_end:
    iref_start:
        dd BE(iref_end - iref_start)
        dd "iref"
        db 0x00 ; "version(8)" 
        db 0x00, 0x00, 0x00 ; "flags(24)" 
        db 0x00, 0x00, 0x00, 0x0E ; "box_size(32)" 
        db 0x61, 0x75, 0x78, 0x6C ; "box_type(32)" ('auxl') 
        db 0x00, 0x03 ; "from_item_ID(16)" 
        db 0x00, 0x01 ; "reference_count(16)" 
        db 0x00, 0x04 ; "to_item_ID(16)" 
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
                db 0x00, 0x00, 0x02, 0x80 ; "image_width(32)" 
                db 0x00, 0x00, 0x01, 0xE0 ; "image_height(32)" 
            ispe_end:
            pasp_start:
                dd BE(pasp_end - pasp_start)
                dd "pasp"
                db 0x00, 0x00, 0x00, 0x01 ; "hSpacing(32)" 
                db 0x00, 0x00, 0x00, 0x01 ; "vSpacing(32)" 
            pasp_end:
            av1C_start:
                dd BE(av1C_end - av1C_start)
                dd "av1C"
                db 0x00, 0x00, 0x00, 0x00
            av1C_end:
            auxC_start:
                dd BE(auxC_end - auxC_start)
                dd "auxC"
                db 0x00 ; "version(8)" 
                db 0x00, 0x00, 0x00 ; "flags(24)" 
                db 0x75 ; "aux_type(8)" ('u') 
                db 0x72 ; "aux_type(8)" ('r') 
                db 0x6E ; "aux_type(8)" ('n') 
                db 0x3A ; "aux_type(8)" (':') 
                db 0x6D ; "aux_type(8)" ('m') 
                db 0x70 ; "aux_type(8)" ('p') 
                db 0x65 ; "aux_type(8)" ('e') 
                db 0x67 ; "aux_type(8)" ('g') 
                db 0x3A ; "aux_type(8)" (':') 
                db 0x6D ; "aux_type(8)" ('m') 
                db 0x70 ; "aux_type(8)" ('p') 
                db 0x65 ; "aux_type(8)" ('e') 
                db 0x67 ; "aux_type(8)" ('g') 
                db 0x42 ; "aux_type(8)" ('B') 
                db 0x3A ; "aux_type(8)" (':') 
                db 0x63 ; "aux_type(8)" ('c') 
                db 0x69 ; "aux_type(8)" ('i') 
                db 0x63 ; "aux_type(8)" ('c') 
                db 0x70 ; "aux_type(8)" ('p') 
                db 0x3A ; "aux_type(8)" (':') 
                db 0x73 ; "aux_type(8)" ('s') 
                db 0x79 ; "aux_type(8)" ('y') 
                db 0x73 ; "aux_type(8)" ('s') 
                db 0x74 ; "aux_type(8)" ('t') 
                db 0x65 ; "aux_type(8)" ('e') 
                db 0x6D ; "aux_type(8)" ('m') 
                db 0x73 ; "aux_type(8)" ('s') 
                db 0x3A ; "aux_type(8)" (':') 
                db 0x61 ; "aux_type(8)" ('a') 
                db 0x75 ; "aux_type(8)" ('u') 
                db 0x78 ; "aux_type(8)" ('x') 
                db 0x69 ; "aux_type(8)" ('i') 
                db 0x6C ; "aux_type(8)" ('l') 
                db 0x69 ; "aux_type(8)" ('i') 
                db 0x61 ; "aux_type(8)" ('a') 
                db 0x72 ; "aux_type(8)" ('r') 
                db 0x79 ; "aux_type(8)" ('y') 
                db 0x3A ; "aux_type(8)" (':') 
                db 0x61 ; "aux_type(8)" ('a') 
                db 0x6C ; "aux_type(8)" ('l') 
                db 0x70 ; "aux_type(8)" ('p') 
                db 0x68 ; "aux_type(8)" ('h') 
                db 0x61 ; "aux_type(8)" ('a') 
                db 0x00 ; "aux_type(8)" 
            auxC_end:
            pixi_start:
                dd BE(pixi_end - pixi_start)
                dd "pixi"
            pixi_end:
            av1C2_start:
                dd BE(av1C2_end - av1C2_start)
                dd "av1C"
                db 0x00, 0x00, 0x00, 0x00
            av1C2_end:
            pixi2_start:
                dd BE(pixi2_end - pixi2_start)
                dd "pixi"
            pixi2_end:
        ipco_end:
        ipma_start:
            dd BE(ipma_end - ipma_start)
            dd "ipma"
            db 0x00 ; "version(8)" 
            db 0x00, 0x00, 0x00 ; "flags(24)" 
            db 0x00, 0x00, 0x00, 0x02 ; "entry_count(32)" 
            db 0x00, 0x03 ; "item_ID(16)" 
            db 0x05 ; "association_count(8)" 
            db 0x81 ; "essential(1)" "property_index(7)" 
            db 0x82 ; "essential(1)" "property_index(7)" 
            db 0x83 ; "essential(1)" "property_index(7)" 
            db 0x84 ; "essential(1)" "property_index(7)" 
            db 0x85 ; "essential(1)" "property_index(7)" 
            db 0x00, 0x04 ; "item_ID(16)" 
            db 0x04 ; "association_count(8)" 
            db 0x81 ; "essential(1)" "property_index(7)" 
            db 0x82 ; "essential(1)" "property_index(7)" 
            db 0x86 ; "essential(1)" "property_index(7)" 
            db 0x87 ; "essential(1)" "property_index(7)" 
        ipma_end:
    iprp_end:
meta_end:
moov_start:
    dd BE(moov_end - moov_start)
    dd "moov"
    mvhd_start:
        dd BE(mvhd_end - mvhd_start)
        dd "mvhd"
    mvhd_end:
    iods_start:
        dd BE(iods_end - iods_start)
        dd "iods"
    iods_end:
    trak_start:
        dd BE(trak_end - trak_start)
        dd "trak"
        tkhd_start:
            dd BE(tkhd_end - tkhd_start)
            dd "tkhd"
            db 0x00 ; "version(8)" 
            db 0x00, 0x00, 0x01 ; "flags(24)" 
            db 0xD8, 0x7E, 0x5C, 0xEE ; "creation_time(32)" 
            db 0xD8, 0x7E, 0x5C, 0xEE ; "modification_time(32)" 
            db 0x00, 0x00, 0x00, 0x01 ; "track_ID(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "reserved(32)" 
            db 0x00, 0x00, 0x04, 0x80 ; "duration(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "reserved(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "reserved(32)" 
            db 0x00, 0x00 ; "layer(16)" 
            db 0x00, 0x00 ; "alternate_group(16)" 
            db 0x00, 0x00 ; "volume(16)" 
            db 0x00, 0x00 ; "reserved(16)" 
            db 0x00, 0x01, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x01, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x40, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x02, 0x80, 0x00, 0x00 ; "width(32)" 
            db 0x01, 0xE0, 0x00, 0x00 ; "height(32)" 
        tkhd_end:
        mdia_start:
            dd BE(mdia_end - mdia_start)
            dd "mdia"
            mdhd_start:
                dd BE(mdhd_end - mdhd_start)
                dd "mdhd"
            mdhd_end:
            hdlr2_start:
                dd BE(hdlr2_end - hdlr2_start)
                dd "hdlr"
                db 0x00 ; "version(8)" 
                db 0x00, 0x00, 0x00 ; "flags(24)" 
                db 0x00, 0x00, 0x00, 0x00 ; "pre_defined(32)" 
                db 0x70, 0x69, 0x63, 0x74 ; "handler_type(32)" ('pict') 
                db 0x00, 0x00, 0x00, 0x00 ; "reserved1(32)" 
                db 0x00, 0x00, 0x00, 0x00 ; "reserved2(32)" 
                db 0x00, 0x00, 0x00, 0x00 ; "reserved3(32)" 
                db 0x47 ; "name(8)" ('G') 
                db 0x50 ; "name(8)" ('P') 
                db 0x41 ; "name(8)" ('A') 
                db 0x43 ; "name(8)" ('C') 
                db 0x20 ; "name(8)" (' ') 
                db 0x61 ; "name(8)" ('a') 
                db 0x76 ; "name(8)" ('v') 
                db 0x69 ; "name(8)" ('i') 
                db 0x66 ; "name(8)" ('f') 
                db 0x73 ; "name(8)" ('s') 
                db 0x00 ; "name(8)" 
            hdlr2_end:
            minf_start:
                dd BE(minf_end - minf_start)
                dd "minf"
                vmhd_start:
                    dd BE(vmhd_end - vmhd_start)
                    dd "vmhd"
                vmhd_end:
                dinf_start:
                    dd BE(dinf_end - dinf_start)
                    dd "dinf"
                    dref_start:
                        dd BE(dref_end - dref_start)
                        dd "dref"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        url_start:
                            dd BE(url_end - url_start)
                            dd "url "
                            db 0x00 ; "version(8)" 
                            db 0x00, 0x00, 0x01 ; "flags(24)" 
                        url_end:
                    dref_end:
                dinf_end:
                stbl_start:
                    dd BE(stbl_end - stbl_start)
                    dd "stbl"
                    stsd_start:
                        dd BE(stsd_end - stsd_start)
                        dd "stsd"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        av01_start:
                            dd BE(av01_end - av01_start)
                            dd "av01"
                            db 0x00 ; "reserved1(8)" 
                            db 0x00 ; "reserved2(8)" 
                            db 0x00 ; "reserved3(8)" 
                            db 0x00 ; "reserved4(8)" 
                            db 0x00 ; "reserved5(8)" 
                            db 0x00 ; "reserved6(8)" 
                            db 0x00, 0x01 ; "data_reference_index(16)" 
                            db 0x00, 0x00 ; "pre_defined(16)" 
                            db 0x00, 0x00 ; "reserved7(16)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "pre_defined1(32)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "pre_defined2(32)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "pre_defined3(32)" 
                            db 0x02, 0x80 ; "width(16)" 
                            db 0x01, 0xE0 ; "height(16)" 
                            db 0x00, 0x48, 0x00, 0x00 ; "horizresolution(32)" 
                            db 0x00, 0x48, 0x00, 0x00 ; "vertresolution(32)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "reserved8(32)" 
                            db 0x00, 0x01 ; "frame_count(16)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00, 0x18 ; "depth(16)" 
                            db 0xFF, 0xFF ; "pre_defined4(16)" 
                            av1C3_start:
                                dd BE(av1C3_end - av1C3_start)
                                dd "av1C"
                                db 0x00, 0x00, 0x00, 0x00
                            av1C3_end:
                            ccst_start:
                                dd BE(ccst_end - ccst_start)
                                dd "ccst"
                            ccst_end:
                            btrt_start:
                                dd BE(btrt_end - btrt_start)
                                dd "btrt"
                            btrt_end:
                        av01_end:
                    stsd_end:
                    stts_start:
                        dd BE(stts_end - stts_start)
                        dd "stts"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        db 0x00, 0x00, 0x00, 0x30 ; "sample_count(32)" 
                        db 0x00, 0x00, 0x03, 0xE8 ; "sample_delta(32)" 
                    stts_end:
                    stss_start:
                        dd BE(stss_end - stss_start)
                        dd "stss"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "sample_number(32)" 
                    stss_end:
                    stsc_start:
                        dd BE(stsc_end - stsc_start)
                        dd "stsc"
                    stsc_end:
                    stsz_start:
                        dd BE(stsz_end - stsz_start)
                        dd "stsz"
                    stsz_end:
                    stco_start:
                        dd BE(stco_end - stco_start)
                        dd "stco"
                    stco_end:
                stbl_end:
            minf_end:
        mdia_end:
    trak_end:
    trak2_start:
        dd BE(trak2_end - trak2_start)
        dd "trak"
        tkhd2_start:
            dd BE(tkhd2_end - tkhd2_start)
            dd "tkhd"
            db 0x00 ; "version(8)" 
            db 0x00, 0x00, 0x01 ; "flags(24)" 
            db 0xD8, 0x7E, 0x5C, 0xEE ; "creation_time(32)" 
            db 0xD8, 0x7E, 0x5C, 0xEE ; "modification_time(32)" 
            db 0x00, 0x00, 0x00, 0x02 ; "track_ID(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "reserved(32)" 
            db 0x00, 0x00, 0x04, 0x80 ; "duration(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "reserved(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "reserved(32)" 
            db 0x00, 0x00 ; "layer(16)" 
            db 0x00, 0x00 ; "alternate_group(16)" 
            db 0x00, 0x00 ; "volume(16)" 
            db 0x00, 0x00 ; "reserved(16)" 
            db 0x00, 0x01, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x01, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x00, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x40, 0x00, 0x00, 0x00 ; "matrix(32)" 
            db 0x02, 0x80, 0x00, 0x00 ; "width(32)" 
            db 0x01, 0xE0, 0x00, 0x00 ; "height(32)" 
        tkhd2_end:
        tref_start:
            dd BE(tref_end - tref_start)
            dd "tref"
            auxl_start:
                dd BE(auxl_end - auxl_start)
                dd "auxl"
                db 0x00, 0x00, 0x00, 0x01 ; "track_IDs(32)" 
            auxl_end:
        tref_end:
        mdia2_start:
            dd BE(mdia2_end - mdia2_start)
            dd "mdia"
            mdhd2_start:
                dd BE(mdhd2_end - mdhd2_start)
                dd "mdhd"
            mdhd2_end:
            hdlr3_start:
                dd BE(hdlr3_end - hdlr3_start)
                dd "hdlr"
                db 0x00 ; "version(8)" 
                db 0x00, 0x00, 0x00 ; "flags(24)" 
                db 0x00, 0x00, 0x00, 0x00 ; "pre_defined(32)" 
                db 0x70, 0x69, 0x63, 0x74 ; "handler_type(32)" ('pict') 
                db 0x00, 0x00, 0x00, 0x00 ; "reserved1(32)" 
                db 0x00, 0x00, 0x00, 0x00 ; "reserved2(32)" 
                db 0x00, 0x00, 0x00, 0x00 ; "reserved3(32)" 
                db 0x47 ; "name(8)" ('G') 
                db 0x50 ; "name(8)" ('P') 
                db 0x41 ; "name(8)" ('A') 
                db 0x43 ; "name(8)" ('C') 
                db 0x20 ; "name(8)" (' ') 
                db 0x61 ; "name(8)" ('a') 
                db 0x76 ; "name(8)" ('v') 
                db 0x69 ; "name(8)" ('i') 
                db 0x66 ; "name(8)" ('f') 
                db 0x73 ; "name(8)" ('s') 
                db 0x20 ; "name(8)" (' ') 
                db 0x61 ; "name(8)" ('a') 
                db 0x6C ; "name(8)" ('l') 
                db 0x70 ; "name(8)" ('p') 
                db 0x68 ; "name(8)" ('h') 
                db 0x61 ; "name(8)" ('a') 
                db 0x00 ; "name(8)" 
            hdlr3_end:
            minf2_start:
                dd BE(minf2_end - minf2_start)
                dd "minf"
                vmhd2_start:
                    dd BE(vmhd2_end - vmhd2_start)
                    dd "vmhd"
                vmhd2_end:
                dinf2_start:
                    dd BE(dinf2_end - dinf2_start)
                    dd "dinf"
                    dref2_start:
                        dd BE(dref2_end - dref2_start)
                        dd "dref"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        url2_start:
                            dd BE(url2_end - url2_start)
                            dd "url "
                            db 0x00 ; "version(8)" 
                            db 0x00, 0x00, 0x01 ; "flags(24)" 
                        url2_end:
                    dref2_end:
                dinf2_end:
                stbl2_start:
                    dd BE(stbl2_end - stbl2_start)
                    dd "stbl"
                    stsd2_start:
                        dd BE(stsd2_end - stsd2_start)
                        dd "stsd"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        av012_start:
                            dd BE(av012_end - av012_start)
                            dd "av01"
                            db 0x00 ; "reserved1(8)" 
                            db 0x00 ; "reserved2(8)" 
                            db 0x00 ; "reserved3(8)" 
                            db 0x00 ; "reserved4(8)" 
                            db 0x00 ; "reserved5(8)" 
                            db 0x00 ; "reserved6(8)" 
                            db 0x00, 0x01 ; "data_reference_index(16)" 
                            db 0x00, 0x00 ; "pre_defined(16)" 
                            db 0x00, 0x00 ; "reserved7(16)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "pre_defined1(32)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "pre_defined2(32)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "pre_defined3(32)" 
                            db 0x02, 0x80 ; "width(16)" 
                            db 0x01, 0xE0 ; "height(16)" 
                            db 0x00, 0x48, 0x00, 0x00 ; "horizresolution(32)" 
                            db 0x00, 0x48, 0x00, 0x00 ; "vertresolution(32)" 
                            db 0x00, 0x00, 0x00, 0x00 ; "reserved8(32)" 
                            db 0x00, 0x01 ; "frame_count(16)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00 ; "compressorname(8)" 
                            db 0x00, 0x18 ; "depth(16)" 
                            db 0xFF, 0xFF ; "pre_defined4(16)" 
                            av1C4_start:
                                dd BE(av1C4_end - av1C4_start)
                                dd "av1C"
                                db 0x00, 0x00, 0x00, 0x00
                            av1C4_end:
                            ccst2_start:
                                dd BE(ccst2_end - ccst2_start)
                                dd "ccst"
                            ccst2_end:
                            auxi_start:
                                dd BE(auxi_end - auxi_start)
                                dd "auxi"
                                db 0x00 ; "version(8)" 
                                db 0x00, 0x00, 0x00 ; "flags(24)" 
                                db 0x75 ; "aux_track_type(8)" ('u') 
                                db 0x72 ; "aux_track_type(8)" ('r') 
                                db 0x6E ; "aux_track_type(8)" ('n') 
                                db 0x3A ; "aux_track_type(8)" (':') 
                                db 0x6D ; "aux_track_type(8)" ('m') 
                                db 0x70 ; "aux_track_type(8)" ('p') 
                                db 0x65 ; "aux_track_type(8)" ('e') 
                                db 0x67 ; "aux_track_type(8)" ('g') 
                                db 0x3A ; "aux_track_type(8)" (':') 
                                db 0x6D ; "aux_track_type(8)" ('m') 
                                db 0x70 ; "aux_track_type(8)" ('p') 
                                db 0x65 ; "aux_track_type(8)" ('e') 
                                db 0x67 ; "aux_track_type(8)" ('g') 
                                db 0x42 ; "aux_track_type(8)" ('B') 
                                db 0x3A ; "aux_track_type(8)" (':') 
                                db 0x63 ; "aux_track_type(8)" ('c') 
                                db 0x69 ; "aux_track_type(8)" ('i') 
                                db 0x63 ; "aux_track_type(8)" ('c') 
                                db 0x70 ; "aux_track_type(8)" ('p') 
                                db 0x3A ; "aux_track_type(8)" (':') 
                                db 0x73 ; "aux_track_type(8)" ('s') 
                                db 0x79 ; "aux_track_type(8)" ('y') 
                                db 0x73 ; "aux_track_type(8)" ('s') 
                                db 0x74 ; "aux_track_type(8)" ('t') 
                                db 0x65 ; "aux_track_type(8)" ('e') 
                                db 0x6D ; "aux_track_type(8)" ('m') 
                                db 0x73 ; "aux_track_type(8)" ('s') 
                                db 0x3A ; "aux_track_type(8)" (':') 
                                db 0x61 ; "aux_track_type(8)" ('a') 
                                db 0x75 ; "aux_track_type(8)" ('u') 
                                db 0x78 ; "aux_track_type(8)" ('x') 
                                db 0x69 ; "aux_track_type(8)" ('i') 
                                db 0x6C ; "aux_track_type(8)" ('l') 
                                db 0x69 ; "aux_track_type(8)" ('i') 
                                db 0x61 ; "aux_track_type(8)" ('a') 
                                db 0x72 ; "aux_track_type(8)" ('r') 
                                db 0x79 ; "aux_track_type(8)" ('y') 
                                db 0x3A ; "aux_track_type(8)" (':') 
                                db 0x61 ; "aux_track_type(8)" ('a') 
                                db 0x6C ; "aux_track_type(8)" ('l') 
                                db 0x70 ; "aux_track_type(8)" ('p') 
                                db 0x68 ; "aux_track_type(8)" ('h') 
                                db 0x61 ; "aux_track_type(8)" ('a') 
                                db 0x00 ; "aux_track_type(8)" 
                            auxi_end:
                            btrt2_start:
                                dd BE(btrt2_end - btrt2_start)
                                dd "btrt"
                            btrt2_end:
                        av012_end:
                    stsd2_end:
                    stts2_start:
                        dd BE(stts2_end - stts2_start)
                        dd "stts"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        db 0x00, 0x00, 0x00, 0x30 ; "sample_count(32)" 
                        db 0x00, 0x00, 0x03, 0xE7 ; "sample_delta(32)" 
                    stts2_end:
                    stss2_start:
                        dd BE(stss2_end - stss2_start)
                        dd "stss"
                        db 0x00 ; "version(8)" 
                        db 0x00, 0x00, 0x00 ; "flags(24)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "entry_count(32)" 
                        db 0x00, 0x00, 0x00, 0x01 ; "sample_number(32)" 
                    stss2_end:
                    stsc2_start:
                        dd BE(stsc2_end - stsc2_start)
                        dd "stsc"
                    stsc2_end:
                    stsz2_start:
                        dd BE(stsz2_end - stsz2_start)
                        dd "stsz"
                    stsz2_end:
                    stco2_start:
                        dd BE(stco2_end - stco2_start)
                        dd "stco"
                    stco2_end:
                stbl2_end:
            minf2_end:
        mdia2_end:
    trak2_end:
moov_end:

; vim: syntax=nasm
