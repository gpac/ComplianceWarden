db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
