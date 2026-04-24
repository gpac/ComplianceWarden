; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Codec Config OBU 1: Valid ipcm
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)

; Codec Config OBU 2: Duplicate codec_config_id = 0
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 0         ; codec_config_id = 0 (duplicate!)
db 'mp4a'    ; codec_id = 'mp4a'
db 100       ; num_samples_per_frame = 100
db 0xFF      ; audio_roll_distance = -1 (valid for mp4a)
db 0xFF

; Codec Config OBU 3: Invalid codec_id
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 1         ; codec_config_id = 1
db 'xxxx'    ; codec_id = 'xxxx' (invalid)
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance
db 0

; Codec Config OBU 4: mp4a with invalid roll distance (0 instead of -1)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 2         ; codec_config_id = 2
db 'mp4a'    ; codec_id = 'mp4a'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance = 0 (invalid for mp4a)
db 0

; Codec Config OBU 5: fLaC with invalid roll distance (-1 instead of 0)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 3         ; codec_config_id = 3
db 'fLaC'    ; codec_id = 'fLaC'
db 100       ; num_samples_per_frame = 100
db 0xFF      ; audio_roll_distance = -1 (invalid for fLaC)
db 0xFF

; Codec Config OBU 6: Opus with invalid roll distance (0 instead of -4)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 9         ; obu_size = 9 bytes
db 4         ; codec_config_id = 4
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0         ; audio_roll_distance = 0 (invalid for Opus)
db 0
