; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Tests "num_substreams SHALL NOT be set to 0."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 6         ; obu_size = 6 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 0         ; num_substreams = 0 -> INVALID!
db 0         ; num_parameters = 0
db 0         ; num_layers = 0

; OBU 2: Tests "When audio_element_type = 0 (CHANNEL_BASED), num_parameters SHALL be set to 0, 1, or 2."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 23        ; obu_size = 23 bytes
db 2         ; audio_element_id = 2
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 3         ; num_parameters = 3 -> INVALID!
; Parameter 1 (Demixing)
db 1         ; param_definition_type = 1 (Demixing)
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; dmixp_mode = 0, reserved = 0
db 0         ; default_w = 0, reserved = 0
; Parameter 2 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 1         ; parameter_id = 1
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
; Parameter 3 (Type 3)
db 3         ; param_definition_type = 3
db 2         ; parameter_id = 2
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 1         ; param_definition_size = 1
db 0         ; param_definition_byte = 0
db 0         ; num_layers = 0

; OBU 3: Tests "When audio_element_type = 1 (SCENE_BASED), num_parameters SHALL be set to 0."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 16        ; obu_size = 16 bytes
db 3         ; audio_element_id = 3
db 00100000b ; audio_element_type = 1 (SCENE_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 1         ; num_parameters = 1 -> INVALID!
; Parameter 1 (Demixing)
db 1         ; param_definition_type = 1 (Demixing)
db 3         ; parameter_id = 3
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; dmixp_mode = 0, reserved = 0
db 0         ; default_w = 0, reserved = 0
db 0         ; ambisonics_mode = 0 (MONO)
db 1         ; output_channel_count = 1
db 1         ; substream_count = 1
db 0         ; channel_mapping = 0

; OBU 4: Tests "The type PARAMETER_DEFINITION_MIX_GAIN SHALL NOT be present in Audio Element OBU."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 11        ; obu_size = 11 bytes
db 5         ; audio_element_id = 5
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 1         ; num_parameters = 1
db 0         ; param_definition_type = 0 (Mix Gain) -> INVALID!
db 5         ; parameter_id = 5
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; num_layers = 0

; OBU 5: Tests "When num_layers > 1, the type PARAMETER_DEFINITION_RECON_GAIN SHALL be present."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 14        ; obu_size = 14 bytes
db 6         ; audio_element_id = 6
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 2         ; num_substreams = 2
db 1         ; audio_substream_id = 1
db 2         ; audio_substream_id = 2
db 0         ; num_parameters = 0 -> INVALID (missing RECON_GAIN)
db 01000000b ; num_layers = 2, reserved = 0
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 2
db 00010000b ; loudspeaker_layout = 1 (Stereo), flags = 0
db 1         ; substream_count = 1
db 1         ; coupled_substream_count = 1

; OBU 6: Tests "When the highest loudspeaker_layout of the scalable channel audio (i.e., num_layers > 1) is greater than 3.1.2ch, both PARAMETER_DEFINITION_DEMIXING and PARAMETER_DEFINITION_RECON_GAIN types SHALL be present."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 18        ; obu_size = 18 bytes
db 7         ; audio_element_id = 7
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 2         ; num_substreams = 2
db 1         ; audio_substream_id = 1
db 2         ; audio_substream_id = 2
db 1         ; num_parameters = 1 -> INVALID (missing DEMIXING)
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 6         ; parameter_id = 6
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 01000000b ; num_layers = 2, reserved = 0
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 2
db 00100000b ; loudspeaker_layout = 2 (5.1ch), flags = 0
db 4         ; substream_count = 4
db 3         ; coupled_substream_count = 2

; OBU 8: Tests "num_layers maximum value SHALL be 6."
obu_8_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_8_end - obu_8_start - 2 ; obu_size
db 8         ; audio_element_id = 8
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 7         ; num_substreams = 7
db 1, 2, 3, 4, 5, 6, 7 ; audio_substream_id list
db 0         ; num_parameters = 0
db 11100000b ; num_layers = 7, reserved = 0
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 2
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 3
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 4
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 5
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 6
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 7
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_8_end:

; OBU 9: Tests "If loudspeaker_layout is set to Binaural, num_layers SHALL be set to 1."
obu_9_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_9_end - obu_9_start - 2 ; obu_size
db 9         ; audio_element_id = 9
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 2         ; num_substreams = 2
db 1, 2      ; audio_substream_id list
db 0         ; num_parameters = 0
db 01000000b ; num_layers = 2, reserved = 0
; Layer 1
db 10010000b ; loudspeaker_layout = 9 (Binaural), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
; Layer 2
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_9_end:

; OBU 10: Tests "substream_count SHALL NOT be set to 0 for any layer."
obu_10_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_10_end - obu_10_start - 2 ; obu_size
db 10        ; audio_element_id = 10
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1, reserved = 0
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 0         ; substream_count = 0
db 0         ; coupled_substream_count = 0
obu_10_end:

; OBU 11: Tests "substream_count SHALL be greater than or equal to coupled_substream_count for all layers."
obu_11_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_11_end - obu_11_start - 2 ; obu_size
db 11        ; audio_element_id = 11
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1, reserved = 0
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 1         ; substream_count = 1
db 2         ; coupled_substream_count = 2
obu_11_end:
