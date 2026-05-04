; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Audio Element OBU with valid scalable layout (Stereo -> 5.1)
obu_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_end - obu_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 4         ; num_substreams = 4
db 1, 2, 3, 4 ; audio_substream_ids
db 2         ; num_parameters = 2
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 5         ; constant_subblock_duration = 5
; Parameter 2 (Demixing)
db 1         ; param_definition_type = 1 (Demixing)
db 3         ; parameter_id = 3
db 0         ; parameter_rate = 0
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 5         ; constant_subblock_duration = 5
db 0         ; dmixp_mode = 0, reserved = 0
db 0         ; default_w = 0, reserved = 0

; ScalableChannelLayoutConfig
db 01000000b ; num_layers = 2, reserved = 0

; Layer 1: Stereo (Layout 1)
db 00010000b ; loudspeaker_layout = 1 (Stereo), flags = 0
db 1         ; substream_count = 1
db 1         ; coupled_substream_count = 1

; Layer 2: 5.1ch (Layout 2)
db 00100000b ; loudspeaker_layout = 2 (5.1), flags = 0
db 3         ; substream_count = 3
db 1         ; coupled_substream_count = 1

obu_end:
