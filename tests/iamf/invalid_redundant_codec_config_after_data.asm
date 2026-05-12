; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Codec Config OBU (Valid ipcm)
obu_codec_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_codec_end - obu_codec_start - 2 ; obu_size
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 64        ; num_samples_per_frame = 64
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)
obu_codec_end:

; OBU 2: Temporal Delimiter OBU (acts as data OBU to set in_data=true)
db 00100000b ; OBU Header: obu_type = 4 (Temporal Delimiter), redundant = 0
db 1         ; obu_size = 1 byte
db 0         ; dummy byte

; OBU 3: Redundant Codec Config OBU (after data)
obu_codec_red_start:
db 00000100b ; OBU Header: obu_type = 0 (Codec Config), redundant = 1
db obu_codec_red_end - obu_codec_red_start - 2 ; obu_size
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 64        ; num_samples_per_frame = 64
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)
obu_codec_red_end:

