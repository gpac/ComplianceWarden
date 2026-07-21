; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile = 0 (Simple)
db 0         ; additional_profile

; Codec Config OBU (Valid ipcm)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0, 0      ; audio_roll_distance = 0 (16 bits)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)

; Audio Element OBU: Valid Ambisonics Config (MONO)
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 00100000b ; audio_element_type = 1 (SCENE_BASED)
db 0         ; codec_config_id = 0
db 4         ; num_substreams = 4
db 1, 2, 3, 4 ; audio_substream_ids
db 0         ; num_parameters = 0
db 0         ; ambisonics_mode = 0 (MONO)
db 4         ; output_channel_count = 4
db 4         ; substream_count = 4
db 0, 1, 2, 3 ; channel_mapping
obu_1_end:

; Mix Presentation OBU (Valid)
obu_3_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_3_end - obu_3_start - 2 ; obu_size
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
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
; output_mix_gain
db 1         ; parameter_id = 1
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
obu_3_end:
