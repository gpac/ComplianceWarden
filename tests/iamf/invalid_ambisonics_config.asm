; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Codec Config OBU: ipcm
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 2         ; codec_config_id = 2
db 'ipcm'    ; codec_id
db 64        ; num_samples_per_frame = 64
db 0, 0      ; audio_roll_distance = 0 (16 bits)

; OBU 1: Tests "ambisonics_mode SHALL be 0 or 1."
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 2         ; ambisonics_mode = 2 -> INVALID!
obu_1_end:

; OBU 2: Tests "output_channel_count SHALL be (1 + n)^2"
obu_2_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_2_end - obu_2_start - 2 ; obu_size
db 2         ; audio_element_id = 2
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 0         ; ambisonics_mode = 0 (MONO)
db 2         ; output_channel_count = 2 -> INVALID!
db 1         ; substream_count = 1
db 0         ; channel_mapping = 0
db 0         ; channel_mapping = 0
obu_2_end:

; OBU 3: Tests "substream_count SHALL be the same as num_substreams in this OBU."
obu_3_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_3_end - obu_3_start - 2 ; obu_size
db 3         ; audio_element_id = 3
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 2         ; num_substreams = 2
db 1, 2      ; audio_substream_ids
db 0         ; num_parameters = 0
db 0         ; ambisonics_mode = 0 (MONO)
db 1         ; output_channel_count = 1
db 1         ; substream_count = 1 -> INVALID (expected 2)
db 0         ; channel_mapping = 0
obu_3_end:

; OBU 4: Tests "For PROJECTION mode, coupled_substream_count SHALL be less than or equal to substream_count."
obu_4_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_4_end - obu_4_start - 2 ; obu_size
db 4         ; audio_element_id = 4
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 1         ; ambisonics_mode = 1 (PROJECTION)
db 1         ; output_channel_count = 1
db 1         ; substream_count = 1
db 2         ; coupled_substream_count = 2 -> INVALID (expected <= 1)
db 0, 0      ; demixing_matrix[0][0] = 0 (16 bits)
db 0, 0      ; demixing_matrix[1][0] = 0 (16 bits)
db 0, 0      ; demixing_matrix[2][0] = 0 (16 bits)
obu_4_end:
