%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24))

_start:
     ; obu(0) 
    db 0x12 ; forbidden(1) obu_type(4) obu_extension_flag(1) obu_has_size_field(1) obu_reserved_1bit(1) 
    db 0x00 ; leb128_byte(8) 
     ; obu(0) 
    db 0x2A ; forbidden(1) ('*') obu_type(4) ('*') obu_extension_flag(1) ('*') obu_has_size_field(1) ('*') obu_reserved_1bit(1) ('*') 
    db 0x06 ; leb128_byte(8) 
    db 0x04 ; leb128_byte(8) 
    db 0xB5 ; itu_t_t35_country_code(8) 
    db 0x00, 0x3C ; itu_t_t35_terminal_provider_code(16) 
    db 0x00, 0x01 ; itu_t_t35_terminal_provider_oriented_code(16) 
     ; obu(0) 
    db 0x32 ; forbidden(1) ('2') obu_type(4) ('2') obu_extension_flag(1) ('2') obu_has_size_field(1) ('2') obu_reserved_1bit(1) ('2') 
    db 0x01 ; leb128_byte(8) 
    db 0x80 ; show_existing_frame(1) frame_to_show_map_idx(3) bits(4) 
_end:

; vim: syntax=nasm
