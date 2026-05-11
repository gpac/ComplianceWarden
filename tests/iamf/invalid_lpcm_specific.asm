; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Invalid sample_format_flags = 2
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 1         ; codec_config_id = 1
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0, 0      ; audio_roll_distance = 0
db 2         ; sample_format_flags = 2 -> INVALID!
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000

; OBU 2: Invalid sample_size = 8
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 2         ; codec_config_id = 2
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0, 0      ; audio_roll_distance = 0
db 1         ; sample_format_flags = 1
db 8         ; sample_size = 8 -> INVALID!
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000

; OBU 3: Invalid sample_rate = 1000
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 3         ; codec_config_id = 3
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0, 0      ; audio_roll_distance = 0
db 1         ; sample_format_flags = 1
db 16        ; sample_size = 16
db 0, 0, 0x03, 0xE8 ; sample_rate = 1000 -> INVALID!
