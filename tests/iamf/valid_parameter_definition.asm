; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Valid parameter definition with mode 0 and constant duration
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 5         ; constant_subblock_duration = 5
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_1_end:

; OBU 2: Valid parameter definition with mode 0 and variable duration
obu_2_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_2_end - obu_2_start - 2 ; obu_size
db 2         ; audio_element_id = 2
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 2         ; audio_substream_id = 2
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 1         ; parameter_id = 1
db 0         ; parameter_rate = 0
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 0         ; constant_subblock_duration = 0
db 2         ; num_subblocks = 2
db 4         ; subblock_duration = 4
db 6         ; subblock_duration = 6 -> SUM = 10 == duration -> VALID!
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_2_end:

; OBU 3: Valid parameter definition with mode 1
obu_3_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_3_end - obu_3_start - 2 ; obu_size
db 3         ; audio_element_id = 3
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 3         ; audio_substream_id = 3
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 2         ; parameter_id = 2
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_3_end:
