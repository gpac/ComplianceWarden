; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Codec Config OBU (Valid Opus)
obu_codec_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_codec_end - obu_codec_start - 2 ; obu_size
db 0         ; codec_config_id = 0
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance (high byte) = -4
db 0xFC      ; audio_roll_distance (low byte)
db 1         ; version = 1
db 2         ; output_channel_count = 2
db 0, 0      ; pre_skip = 0
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 0         ; channel_mapping_family = 0
obu_codec_end:

; Audio Element OBU with invalid scalable layout (5.1 defined before Stereo, violating non-decreasing order)
obu_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_end - obu_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 5         ; num_substreams = 5
db 1, 2, 3, 4, 5 ; audio_substream_ids
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain)
db 2         ; param_definition_type = 2 (Recon Gain)
db 0         ; parameter_id = 0
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 00000000b ; param_definition_mode = 0, reserved = 0
db 10        ; duration = 10
db 5         ; constant_subblock_duration = 5

; ScalableChannelLayoutConfig
db 01000000b ; num_layers = 2, reserved = 0

; Layer 1: 5.1ch (Layout 2)
db 00100000b ; loudspeaker_layout = 2 (5.1), flags = 0
db 4         ; substream_count = 4
db 2         ; coupled_substream_count = 2

; Layer 2: Stereo (Layout 1)
db 00010000b ; loudspeaker_layout = 1 (Stereo), flags = 0
db 1         ; substream_count = 1
db 1         ; coupled_substream_count = 1

obu_end:

; Mix Presentation OBU (Valid)
obu_mix_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_mix_end - obu_mix_start - 2 ; obu_size
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
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0, 0      ; default_mix_gain = 0
; output_mix_gain
db 1         ; parameter_id = 1
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0, 0      ; default_mix_gain = 0
db 1         ; num_layouts = 1
; Layout 1
db 10000000b ; layout_type = 2 (LOUDSPEAKERS_SS_CONVENTION), sound_system = 0 (Stereo), reserved = 0
; Loudness
db 0         ; info_type = 0
db 0, 0      ; integrated_loudness = 0
db 0, 0      ; digital_peak = 0
obu_mix_end:
