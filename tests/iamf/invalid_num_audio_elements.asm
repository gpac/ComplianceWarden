; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Codec Config OBU (Valid ipcm)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xAC, 0x44 ; sample_rate = 44100 (big-endian)

; Audio Element OBU (Valid)
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 10        ; obu_size = 10 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1
db 0         ; loudspeaker_layout = 0 (Mono)
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0

; Mix Presentation OBU (Invalid: num_audio_elements = 29)
obu_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db ((obu_end - obu_start - 3) & 0x7F) | 0x80 ; obu_size (LEB128 low byte)
db (obu_end - obu_start - 3) >> 7   ; obu_size (LEB128 high byte)
db 1         ; mix_presentation_id = 1
db 0         ; count_label = 0
db 1         ; num_sub_mixes = 1
db 29        ; num_audio_elements = 29

; Element 1
db 1         ; audio_element_id = 1
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 100       ; parameter_id = 100
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 2
db 2         ; audio_element_id = 2
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 101       ; parameter_id = 101
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 3
db 3         ; audio_element_id = 3
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 102       ; parameter_id = 102
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 4
db 4         ; audio_element_id = 4
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 103       ; parameter_id = 103
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 5
db 5         ; audio_element_id = 5
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 104       ; parameter_id = 104
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 6
db 6         ; audio_element_id = 6
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 105       ; parameter_id = 105
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 7
db 7         ; audio_element_id = 7
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 106       ; parameter_id = 106
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 8
db 8         ; audio_element_id = 8
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 107       ; parameter_id = 107
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 9
db 9         ; audio_element_id = 9
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 108       ; parameter_id = 108
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 10
db 10        ; audio_element_id = 10
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 109       ; parameter_id = 109
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 11
db 11        ; audio_element_id = 11
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 110       ; parameter_id = 110
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 12
db 12        ; audio_element_id = 12
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 111       ; parameter_id = 111
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 13
db 13        ; audio_element_id = 13
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 112       ; parameter_id = 112
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 14
db 14        ; audio_element_id = 14
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 113       ; parameter_id = 113
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 15
db 15        ; audio_element_id = 15
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 114       ; parameter_id = 114
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 16
db 16        ; audio_element_id = 16
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 115       ; parameter_id = 115
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 17
db 17        ; audio_element_id = 17
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 116       ; parameter_id = 116
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 18
db 18        ; audio_element_id = 18
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 117       ; parameter_id = 117
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 19
db 19        ; audio_element_id = 19
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 118       ; parameter_id = 118
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 20
db 20        ; audio_element_id = 20
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 119       ; parameter_id = 119
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 21
db 21        ; audio_element_id = 21
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 120       ; parameter_id = 120
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 22
db 22        ; audio_element_id = 22
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 121       ; parameter_id = 121
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 23
db 23        ; audio_element_id = 23
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 122       ; parameter_id = 122
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 24
db 24        ; audio_element_id = 24
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 123       ; parameter_id = 123
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 25
db 25        ; audio_element_id = 25
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 124       ; parameter_id = 124
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 26
db 26        ; audio_element_id = 26
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 125       ; parameter_id = 125
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 27
db 27        ; audio_element_id = 27
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 126       ; parameter_id = 126
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 28
db 28        ; audio_element_id = 28
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 127       ; parameter_id = 127
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; Element 29
db 29        ; audio_element_id = 29
db 0         ; headphones_rendering_mode = 0
db 0         ; rendering_config_extension_size = 0
db 0x80, 0x01 ; parameter_id = 128
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0

; output_mix_gain
db 0x81, 0x01 ; parameter_id = 129
db 0xC4, 0xD8, 0x02 ; parameter_rate = 44100
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
db 1         ; num_layouts = 1
; Layout 1
db 10000000b ; layout_type = 2, sound_system = 0 (Stereo)
; Loudness
db 0         ; info_type = 0
db 0, 0      ; integrated_loudness = 0
db 0, 0      ; digital_peak = 0
obu_end:
