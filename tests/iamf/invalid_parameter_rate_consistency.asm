; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

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

; Audio Element OBU (Non-scalable Stereo)
obu_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_end - obu_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_ids = 1
db 0         ; num_parameters = 0

; ScalableChannelLayoutConfig
db 00100000b ; num_layers = 1, reserved = 0

; Layer 1: Stereo (Layout 1)
db 00010000b ; loudspeaker_layout = 1 (Stereo), flags = 0
db 1         ; substream_count = 1
db 1         ; coupled_substream_count = 1

obu_end:

; Mix Presentation OBU (Inconsistent parameter rate for parameter_id 0)
mix_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db mix_end - mix_start - 2 ; obu_size
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
db 0xC0, 0xBB, 0x01 ; parameter_rate = 24000
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0, 0      ; default_mix_gain = 0
; output_mix_gain
db 0         ; parameter_id = 0 (Same as above, inconsistent rate)
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
mix_end:
