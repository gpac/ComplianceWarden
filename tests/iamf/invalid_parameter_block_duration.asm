; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Audio Element OBU (Valid)
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 10        ; obu_size = 10 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), output_gain_is_present_flag = 0, recon_gain_is_present_flag = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0

; Mix Presentation OBU (Valid)
obu_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_end - obu_start - 2 ; obu_size
db 1         ; mix_presentation_id = 1
db 0         ; count_label = 0
db 1         ; num_sub_mixes = 1
; Sub-mix 1
db 1         ; num_audio_elements = 1
db 1         ; audio_element_id = 1
db 0         ; headphones_rendering_mode = 0, reserved = 0
db 0         ; rendering_config_extension_size = 0
; element_mix_gain
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0, 0      ; default_mix_gain = 0
; output_mix_gain
db 1         ; parameter_id = 1
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0, 0      ; default_mix_gain = 0
db 1         ; num_layouts = 1
; Layout 1
db 10000000b ; layout_type = 2 (LOUDSPEAKERS_SS_CONVENTION), sound_system = 0 (Stereo), reserved = 0
; Loudness
db 0         ; info_type = 0
db 0, 0      ; integrated_loudness = 0
db 0, 0      ; digital_peak = 0
obu_end:

; Parameter Block OBU (Invalid: duration = 0)
db 00011000b ; OBU Header: obu_type = 3 (Parameter Block)
db 9         ; obu_size = 9 bytes
db 0         ; parameter_id = 0
db 0         ; duration = 0 -> INVALID!
db 5         ; constant_subblock_duration = 5
; Subblock 1
db 0         ; animation_type = 0, reserved = 0
db 0, 0      ; start_point_value = 0
; Subblock 2
db 0         ; animation_type = 0, reserved = 0
db 0, 0      ; start_point_value = 0

; Parameter Block OBU (Invalid: subblock_duration = 0)
db 00011000b ; OBU Header: obu_type = 3 (Parameter Block)
db 8         ; obu_size = 8 bytes
db 0         ; parameter_id = 0
db 10        ; duration = 10
db 0         ; constant_subblock_duration = 0
db 1         ; num_subblocks = 1
db 0         ; subblock_duration = 0 -> INVALID!
db 0         ; animation_type = 0
db 0, 0      ; start_point_value = 0

; Parameter Block OBU (Invalid: summation of subblock_duration != duration)
db 00011000b ; OBU Header: obu_type = 3 (Parameter Block)
db 12        ; obu_size = 12 bytes
db 0         ; parameter_id = 0
db 10        ; duration = 10
db 0         ; constant_subblock_duration = 0
db 2         ; num_subblocks = 2
; Subblock 1
db 4         ; subblock_duration = 4
db 0         ; animation_type = 0
db 0, 0      ; start_point_value = 0
; Subblock 2
db 4         ; subblock_duration = 4 (Sum = 8 != duration 10) -> INVALID!
db 0         ; animation_type = 0
db 0, 0      ; start_point_value = 0
