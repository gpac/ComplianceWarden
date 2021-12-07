%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

ftyp_start:
    dd BE(ftyp_end - ftyp_start)
    dd "ftyp"
    db 0x61, 0x76, 0x69, 0x73 ; brand(32) ('avis') 
    db 0x00, 0x00, 0x00, 0x00 ; version(32) 
    db 0x61, 0x76, 0x69, 0x73 ; compatible_brand(32) ('avis') 
    db 0x6D, 0x73, 0x66, 0x31 ; compatible_brand(32) ('msf1') 
    db 0x69, 0x73, 0x6F, 0x38 ; compatible_brand(32) ('iso8') 
    db 0x6D, 0x69, 0x61, 0x66 ; compatible_brand(32) ('miaf') 
    db 0x6D, 0x69, 0x66, 0x31 ; compatible_brand(32) ('mif1') 
    db 0x4D, 0x41, 0x31, 0x42 ; compatible_brand(32) ('MA1B') 
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
    iloc_start:
        dd BE(iloc_end - iloc_start)
        dd "iloc"
        db 0x01 ; version(8) 
        db 0x00, 0x00, 0x00 ; flags(24) 
        db 0x04 ; offset_size(4) length_size(4) 
        db 0x40 ; base_offset_size(4) ('@') reserved1(4) ('@') 
        db 0x00, 0x01 ; item_count(16) 
        db 0x00, 0x01 ; item_ID(16) 
        db 0x00, 0x00 ; "reserved2(12)" "construction_method(4)" 
        db 0x00, 0x00 ; data_reference_index(16) 
        db 0x00, 0x00, 0x03, 0x65 ; base_offset(32) 
        db 0x00, 0x01 ; extent_count(16) 
         ; extent_offset(0) 
        db 0x00, 0x00, 0x00, 0x0D ; extent_length(32) 
    iloc_end:
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
            db 0x61, 0x76, 0x30, 0x31 ; item_type(32) ('av01') 
            db 0x49 ; item_name(8) ('I') 
            db 0x6D ; item_name(8) ('m') 
            db 0x61 ; item_name(8) ('a') 
            db 0x67 ; item_name(8) ('g') 
            db 0x65 ; item_name(8) ('e') 
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
                db 0x00, 0x00, 0x01, 0xE0 ; image_width(32) 
                db 0x00, 0x00, 0x01, 0x0E ; image_height(32) 
            ispe_end:
            pasp_start:
                dd BE(pasp_end - pasp_start)
                dd "pasp"
                db 0x00, 0x00, 0x00, 0x01 ; hSpacing(32) 
                db 0x00, 0x00, 0x00, 0x01 ; vSpacing(32) 
            pasp_end:
            av1C_start:
                dd BE(av1C_end - av1C_start)
                dd "av1C"
                db 0x81 ; marker(1) version(7) 
                db 0x40 ; seq_profile(3) seq_level_idx_0(5) 
                db 0x48 ; seq_tier_0(1) ('L') high_bitdepth(1) ('L') twelve_bit(1) ('L') monochrome(1) ('L') chroma_subsampling_x(1) ('L') chroma_subsampling_y(1) ('L') chroma_sample_position(2) ('L') 
                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
            av1C_end:
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
moov_start:
    dd BE(moov_end - moov_start)
    dd "moov"
    mvhd_start:
        dd BE(mvhd_end - mvhd_start)
        dd "mvhd"
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0xD8 ; (8) 
        db 0x7E ; (8) ('~') 
        db 0xD7 ; (8) 
        db 0x51 ; (8) ('Q') 
        db 0xD8 ; (8) 
        db 0x7E ; (8) ('~') 
        db 0xD7 ; (8) 
        db 0x51 ; (8) ('Q') 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x02 ; (8) 
        db 0x58 ; (8) ('X') 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x09 ; (8) 
        db 0x49 ; (8) ('I') 
        db 0x00 ; (8) 
        db 0x01 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x01 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x01 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x01 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x40 ; (8) ('@') 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x03 ; (8) 
    mvhd_end:
    iods_start:
        dd BE(iods_end - iods_start)
        dd "iods"
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x00 ; (8) 
        db 0x10 ; (8) 
        db 0x07 ; (8) 
        db 0x00 ; (8) 
        db 0x4F ; (8) ('O') 
        db 0xFF ; (8) 
        db 0xFF ; (8) 
        db 0xFF ; (8) 
        db 0xFE ; (8) 
        db 0xFF ; (8) 
    iods_end:
    trak_start:
        dd BE(trak_end - trak_start)
        dd "trak"
        tkhd_start:
            dd BE(tkhd_end - tkhd_start)
            dd "tkhd"
            db 0x00 ; version(8) 
            db 0x00, 0x00, 0x01 ; flags(24) 
            db 0xD7, 0xAE, 0x43, 0xC0 ; creation_time(32) 
            db 0xD8, 0x7E, 0xD7, 0x51 ; modification_time(32) 
            db 0x00, 0x00, 0x00, 0x02 ; track_ID(32) 
            db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
            db 0x00, 0x00, 0x09, 0x49 ; duration(32) 
            db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
            db 0x00, 0x00, 0x00, 0x00 ; reserved(32) 
            db 0x00, 0x00 ; layer(16) 
            db 0x00, 0x00 ; alternate_group(16) 
            db 0x00, 0x00 ; volume(16) 
            db 0x00, 0x00 ; reserved(16) 
            db 0x00, 0x01, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x01, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x00, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x40, 0x00, 0x00, 0x00 ; matrix(32) 
            db 0x01, 0xE0, 0x00, 0x00 ; width(32) 
            db 0x01, 0x0E, 0x00, 0x00 ; height(32) 
        tkhd_end:
        mdia_start:
            dd BE(mdia_end - mdia_start)
            dd "mdia"
            mdhd_start:
                dd BE(mdhd_end - mdhd_start)
                dd "mdhd"
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0xD7 ; (8) 
                db 0xAE ; (8) 
                db 0x43 ; (8) ('C') 
                db 0xC0 ; (8) 
                db 0xD8 ; (8) 
                db 0x7E ; (8) ('~') 
                db 0xD7 ; (8) 
                db 0x51 ; (8) ('Q') 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
                db 0x5D ; (8) (']') 
                db 0xC0 ; (8) 
                db 0x00 ; (8) 
                db 0x01 ; (8) 
                db 0x73 ; (8) ('s') 
                db 0x77 ; (8) ('w') 
                db 0x55 ; (8) ('U') 
                db 0xC4 ; (8) 
                db 0x00 ; (8) 
                db 0x00 ; (8) 
            mdhd_end:
            hdlr2_start:
                dd BE(hdlr2_end - hdlr2_start)
                dd "hdlr"
                db 0x00 ; version(8) 
                db 0x00, 0x00, 0x00 ; flags(24) 
                db 0x00, 0x00, 0x00, 0x00 ; pre_defined(32) 
                db 0x70, 0x69, 0x63, 0x74 ; handler_type(32) ('pict') 
                db 0x00, 0x00, 0x00, 0x00 ; reserved1(32) 
                db 0x00, 0x00, 0x00, 0x00 ; reserved2(32) 
                db 0x00, 0x00, 0x00, 0x00 ; reserved3(32) 
                db 0x47 ; name(8) ('G') 
                db 0x50 ; name(8) ('P') 
                db 0x41 ; name(8) ('A') 
                db 0x43 ; name(8) ('C') 
                db 0x20 ; name(8) (' ') 
                db 0x61 ; name(8) ('a') 
                db 0x76 ; name(8) ('v') 
                db 0x69 ; name(8) ('i') 
                db 0x66 ; name(8) ('f') 
                db 0x73 ; name(8) ('s') 
                db 0x00 ; name(8) 
            hdlr2_end:
            minf_start:
                dd BE(minf_end - minf_start)
                dd "minf"
                vmhd_start:
                    dd BE(vmhd_end - vmhd_start)
                    dd "vmhd"
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x01 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                    db 0x00 ; (8) 
                vmhd_end:
                dinf_start:
                    dd BE(dinf_end - dinf_start)
                    dd "dinf"
                    dref_start:
                        dd BE(dref_end - dref_start)
                        dd "dref"
                        db 0x00 ; version(8) 
                        db 0x00, 0x00, 0x00 ; flags(24) 
                        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
                        url_start:
                            dd BE(url_end - url_start)
                            dd "url "
                            db 0x00 ; version(8) 
                            db 0x00, 0x00, 0x01 ; flags(24) 
                        url_end:
                    dref_end:
                dinf_end:
                stbl_start:
                    dd BE(stbl_end - stbl_start)
                    dd "stbl"
                    stsd_start:
                        dd BE(stsd_end - stsd_start)
                        dd "stsd"
                        db 0x00 ; version(8) 
                        db 0x00, 0x00, 0x00 ; flags(24) 
                        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
                        av01_start:
                            dd BE(av01_end - av01_start)
                            dd "av01"
                            db 0x00 ; reserved1(8) 
                            db 0x00 ; reserved2(8) 
                            db 0x00 ; reserved3(8) 
                            db 0x00 ; reserved4(8) 
                            db 0x00 ; reserved5(8) 
                            db 0x00 ; reserved6(8) 
                            db 0x00, 0x01 ; data_reference_index(16) 
                            db 0x00, 0x00 ; pre_defined(16) 
                            db 0x00, 0x00 ; reserved7(16) 
                            db 0x00, 0x00, 0x00, 0x00 ; pre_defined1(32) 
                            db 0x00, 0x00, 0x00, 0x00 ; pre_defined2(32) 
                            db 0x00, 0x00, 0x00, 0x00 ; pre_defined3(32) 
                            db 0x01, 0xE0 ; width(16) 
                            db 0x01, 0x0E ; height(16) 
                            db 0x00, 0x48, 0x00, 0x00 ; horizresolution(32) 
                            db 0x00, 0x48, 0x00, 0x00 ; vertresolution(32) 
                            db 0x00, 0x00, 0x00, 0x00 ; reserved8(32) 
                            db 0x00, 0x01 ; frame_count(16) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00 ; compressorname(8) 
                            db 0x00, 0x18 ; depth(16) 
                            db 0xFF, 0xFF ; pre_defined4(16) 
                            av1C2_start:
                                dd BE(av1C2_end - av1C2_start)
                                dd "av1C"
                                db 0x81 ; marker(1) version(7) 
                                db 0x40 ; seq_profile(3) seq_level_idx_0(5) 
                                db 0x4C ; seq_tier_0(1) ('L') high_bitdepth(1) ('L') twelve_bit(1) ('L') monochrome(1) ('L') chroma_subsampling_x(1) ('L') chroma_subsampling_y(1) ('L') chroma_sample_position(2) ('L') 
                                db 0x00 ; reserved(3) initial_presentation_delay_present(1) reserved(4) 
                            av1C2_end:
                            ccst_start:
                                dd BE(ccst_end - ccst_start)
                                dd "ccst"
                                db 0x00 ; (8) 
                                db 0x00 ; (8) 
                                db 0x00 ; (8) 
                                db 0x00 ; (8) 
                                db 0x7C ; (8) ('|') 
                                db 0x00 ; (8) 
                                db 0x00 ; (8) 
                                db 0x00 ; (8) 
                            ccst_end:
                            btrt_start:
                                dd BE(btrt_end - btrt_start)
                                dd "btrt"
                                db 0x00 ; (8) 
                                db 0x00 ; (8) 
                                db 0x50 ; (8) ('P') 
                                db 0x1F ; (8) 
                                db 0x00 ; (8) 
                                db 0x06 ; (8) 
                                db 0xF0 ; (8) 
                                db 0x98 ; (8) 
                                db 0x00 ; (8) 
                                db 0x04 ; (8) 
                                db 0x64 ; (8) ('d') 
                                db 0x30 ; (8) ('0') 
                            btrt_end:
                        av01_end:
                    stsd_end:
                    stts_start:
                        dd BE(stts_end - stts_start)
                        dd "stts"
                        db 0x00 ; version(8) 
                        db 0x00, 0x00, 0x00 ; flags(24) 
                        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
                        db 0x00, 0x00, 0x00, 0x5F ; sample_count(32) 
                        db 0x00, 0x00, 0x03, 0xE9 ; sample_delta(32) 
                    stts_end:
                    stss_start:
                        dd BE(stss_end - stss_start)
                        dd "stss"
                        db 0x00 ; version(8) 
                        db 0x00, 0x00, 0x00 ; flags(24) 
                        db 0x00, 0x00, 0x00, 0x01 ; entry_count(32) 
                        db 0x00, 0x00, 0x00, 0x01 ; sample_number(32) 
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
                        db 0x00 ; version(8) 
                        db 0x00, 0x00, 0x00 ; flags(24) 
                        db 0x00, 0x00, 0x00, 0x00 ; entry_count(32) 
                    stco_end:
                stbl_end:
            minf_end:
        mdia_end:
    trak_end:
moov_end:
mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
    db 0x0A ; (8) 
    db 0x0B ; (8) 
    db 0x40 ; (8) 
    db 0x00 ; (8) 
    db 0x00 ; (8) 
    db 0x04 ; (8) 
    db 0x47 ; (8) ('G') 
    db 0x7E ; (8) ('~') 
    db 0x1A ; (8) 
    db 0xFF ; (8) 
    db 0xFC ; (8) 
    db 0xE0 ; (8) 
    db 0x60 ; (8) ('`') 
    db 0x32 ; (8) ('2') 
    db 0x8E ; (8) 
mdat_end:

; vim: syntax=nasm
