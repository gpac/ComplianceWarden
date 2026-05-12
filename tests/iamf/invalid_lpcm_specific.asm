; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Invalid sample_format_flags = 2
obu_1_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; codec_config_id = 1
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 2         ; sample_format_flags = 2 -> INVALID!
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000
obu_1_end:

; OBU 2: Invalid sample_size = 8
obu_2_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_2_end - obu_2_start - 2 ; obu_size
db 2         ; codec_config_id = 2
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1
db 8         ; sample_size = 8 -> INVALID!
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000
obu_2_end:

; OBU 3: Invalid sample_rate = 1000
obu_3_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_3_end - obu_3_start - 2 ; obu_size
db 3         ; codec_config_id = 3
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1
db 16        ; sample_size = 16
db 0, 0, 0x03, 0xE8 ; sample_rate = 1000 -> INVALID!
obu_3_end:

; OBU 4: Valid Codec Config (ipcm)
obu_4_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_4_end - obu_4_start - 2 ; obu_size
db 4         ; codec_config_id = 4
db 'ipcm'    ; codec_id = 'ipcm'
db 64        ; num_samples_per_frame = 64
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)
obu_4_end:

; OBU 5: Valid Audio Element
obu_5_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_5_end - obu_5_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 4         ; codec_config_id = 4 (References OBU 4)
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1, reserved = 0
db 0         ; loudspeaker_layout = 0 (Mono), output_gain_is_present_flag = 0, recon_gain_is_present_flag = 0
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_5_end:

; OBU 6: Valid Mix Presentation
obu_6_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_6_end - obu_6_start - 2 ; obu_size
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
obu_6_end:


