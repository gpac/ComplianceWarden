; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Audio Element OBU: Valid Ambisonics Config (MONO)
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 0         ; ambisonics_mode = 0 (MONO)
db 1         ; output_channel_count = 1
db 1         ; substream_count = 1
db 0         ; channel_mapping = 0
obu_1_end:

; Audio Element OBU: Valid Ambisonics Config (PROJECTION)
obu_2_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_2_end - obu_2_start - 2 ; obu_size
db 2         ; audio_element_id = 2
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 2         ; num_substreams = 2
db 2, 3      ; audio_substream_ids
db 0         ; num_parameters = 0
db 1         ; ambisonics_mode = 1 (PROJECTION)
db 4         ; output_channel_count = 4
db 2         ; substream_count = 2
db 1         ; coupled_substream_count = 1
; Demixing matrix: 3 rows, 4 cols -> 12 elements. Next line defines 12 16-bit zeros (24 bytes).
times 12 db 0, 0
obu_2_end:
