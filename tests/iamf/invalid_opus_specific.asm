; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Opus with invalid version (2 instead of 1)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 0         ; codec_config_id = 0
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance = -4 (valid for Opus)
db 0xFC
; DecoderConfig for Opus
db 2         ; version = 2 -> INVALID!
db 2         ; output_channel_count = 2
db 0x01, 0x38 ; pre_skip = 312
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 0         ; channel_mapping_family = 0

; OBU 2: Opus with invalid channel count (1 instead of 2)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 1         ; codec_config_id = 1
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance = -4
db 0xFC
; DecoderConfig for Opus
db 1         ; version = 1
db 1         ; output_channel_count = 1 -> INVALID!
db 0x01, 0x38 ; pre_skip = 312
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 0         ; channel_mapping_family = 0

; OBU 3: Opus with invalid output gain (1 instead of 0)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 2         ; codec_config_id = 2
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance = -4
db 0xFC
; DecoderConfig for Opus
db 1         ; version = 1
db 2         ; output_channel_count = 2
db 0x01, 0x38 ; pre_skip = 312
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 1      ; output_gain = 1 -> INVALID!
db 0         ; channel_mapping_family = 0

; OBU 4: Opus with invalid channel mapping family (1 instead of 0)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 3         ; codec_config_id = 3
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance = -4
db 0xFC
; DecoderConfig for Opus
db 1         ; version = 1
db 2         ; output_channel_count = 2
db 0x01, 0x38 ; pre_skip = 312
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 1         ; channel_mapping_family = 1 -> INVALID!

; OBU 5: Opus with invalid pre_skip (312 but no matching Audio Frame OBUs)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 4         ; codec_config_id = 4
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0xFF      ; audio_roll_distance = -4
db 0xFC
; DecoderConfig for Opus
db 1         ; version = 1
db 2         ; output_channel_count = 2
db 0x01, 0x38 ; pre_skip = 312 -> INVALID (no matching frames are trimmed)
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 0         ; channel_mapping_family = 0

; OBU 6: Audio Element referencing Codec Config 4 (Opus with invalid pre_skip)
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 10        ; obu_size = 10 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 4         ; codec_config_id = 4
db 1         ; num_substreams = 1
db 5         ; audio_substream_id = 5
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono)
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0

obu_7_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_7_end - obu_7_start - 2 ; obu_size
db 1         ; mix_presentation_id = 1
db 0         ; count_label = 0
db 1         ; num_sub_mixes = 1
; Sub-mix 1
db 1         ; num_audio_elements = 1
db 1         ; audio_element_id = 1
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
; element_mix_gain
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
; output_mix_gain
db 1         ; parameter_id = 1
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
db 1         ; num_layouts = 1
; Layout 1
db 10000000b ; layout_type = 2, sound_system = 0 (Stereo)
; Loudness
db 0         ; info_type = 0
db 0, 0      ; integrated_loudness = 0
db 0, 0      ; digital_peak = 0
obu_7_end:
