; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 1         ; primary_profile = 1 (Base)
db 1         ; additional_profile = 1 (Base)

; Codec Config OBU (Valid Opus)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 0         ; codec_config_id = 0
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance = -4
db 0xFC
; DecoderConfig for Opus
db 1         ; version = 1
db 2         ; output_channel_count = 2
db 0x00, 0x00 ; pre_skip = 0
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 0         ; channel_mapping_family = 0

; Audio Element OBU 1
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 10        ; num_substreams = 10
db 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ; audio_substream_ids
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 1         ; parameter_id = 1
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 0         ; constant_subblock_duration = 0
db 2         ; num_subblocks = 2
db 4         ; subblock_duration = 4
db 6         ; subblock_duration = 6
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 10        ; substream_count = 10 (INVALID: Mono expects 1)
db 0         ; coupled_substream_count = 0
obu_1_end:

; Audio Element OBU 2
obu_2_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_2_end - obu_2_start - 2 ; obu_size
db 2         ; audio_element_id = 2
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 10        ; num_substreams = 10
db 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 ; audio_substream_ids
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 2         ; parameter_id = 2
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1, reserved = 0
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), flags = 0
db 10        ; substream_count = 10 (INVALID: Mono expects 1)
db 0         ; coupled_substream_count = 0
obu_2_end:

; Mix Presentation OBU
; INVALID: The sum of channels across all Audio Elements in this Mix Presentation
; before mixing is 10 + 10 = 20, which exceeds the Base Profile limit of 18.
obu_mix_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_mix_end - obu_mix_start - 2 ; obu_size
db 1         ; mix_presentation_id = 1
db 0         ; count_label = 0
db 1         ; num_sub_mixes = 1
; Sub-mix 1
db 2         ; num_audio_elements = 2
db 1         ; audio_element_id = 1
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
; element_mix_gain
db 0         ; parameter_id = 0
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
; Next element
db 2         ; audio_element_id = 2
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
; element_mix_gain
db 3         ; parameter_id = 3
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; output_mix_gain
db 4         ; parameter_id = 4
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
db 1         ; num_layouts = 1
; Layout 1
db 10000000b ; layout_type = 2, sound_system = 0 (Stereo)
; Loudness
db 0         ; info_type = 0
db 0, 0      ; integrated_loudness = 0
db 0, 0      ; digital_peak = 0
obu_mix_end:
