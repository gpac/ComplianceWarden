; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Audio Element OBU: Tests "The parameter type SHALL NOT be duplicated in one Audio Element OBU."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 19        ; obu_size = 19 bytes
db 0         ; audio_element_id = 0
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 2         ; num_parameters = 2
; Parameter 1
db 1         ; param_definition_type = 1 (Demixing)
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; dmixp_mode = 0, reserved = 0
db 0         ; default_w = 0, reserved = 0
; Parameter 2
db 1         ; param_definition_type = 1 (Demixing) -> DUPLICATED!
db 1         ; parameter_id = 1
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; dmixp_mode = 0, reserved = 0
db 0         ; default_w = 0, reserved = 0
db 0         ; num_layers = 0 (for ScalableChannelLayoutConfig)
