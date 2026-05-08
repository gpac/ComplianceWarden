; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Tests "There SHALL be one unique parameter_id per Parameter Substream."
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 2         ; num_parameters = 2
; Parameter 1 (Demixing)
db 1         ; param_definition_type = 1 (Demixing)
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; dmixp_mode = 0, reserved = 0
db 0         ; default_w = 0, reserved = 0
; Parameter 2 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 0         ; parameter_id = 0 -> DUPLICATE!
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; num_layers = 0
obu_1_end:

; OBU 2: Tests "duration SHALL NOT be set to 0."
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
db 0         ; duration = 0 -> INVALID!
db 1         ; constant_subblock_duration = 1
db 0         ; num_layers = 0
obu_2_end:

; OBU 3: Tests "subblock_duration SHALL NOT be set to 0."
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
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 0         ; constant_subblock_duration = 0
db 2         ; num_subblocks = 2
db 0         ; subblock_duration = 0 -> INVALID!
db 10        ; subblock_duration = 10
db 0         ; num_layers = 0
obu_3_end:

; OBU 4: Tests "summation of all subblock_duration ... SHALL be equal to duration."
obu_4_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_4_end - obu_4_start - 2 ; obu_size
db 4         ; audio_element_id = 4
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 4         ; audio_substream_id = 4
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 3         ; parameter_id = 3
db 0         ; parameter_rate = 0
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 0         ; constant_subblock_duration = 0
db 2         ; num_subblocks = 2
db 4         ; subblock_duration = 4
db 4         ; subblock_duration = 4 -> SUM = 8 != 10 -> INVALID!
db 0         ; num_layers = 0
obu_4_end:
