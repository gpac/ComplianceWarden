; Tests missing required descriptor OBUs before new Sequence Header.
db 11111000b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Codec Config OBU
db 00000000b ; OBU Header: obu_type = 0 (Codec Config), redundant = 0
db 14        ; obu_size = 14 bytes
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)

; OBU 2: Redundant IA Sequence Header OBU (missing Audio Element and Mix Presentation)
db 11111100b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant = 1
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile
