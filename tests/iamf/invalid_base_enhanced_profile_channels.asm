; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 2         ; primary_profile = 2 (Base Enhanced)
db 2         ; additional_profile = 2 (Base Enhanced)

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

; Audio Element OBU (Ambisonics 36 channels)
obu_1_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_1_end - obu_1_payload ; obu_size
obu_1_payload:
db 1         ; audio_element_id = 1
db 00100000b ; audio_element_type = 1 (SCENE_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 36        ; num_substreams = 36
; 36 substream IDs (1 to 36)
%assign i 1
%rep 36
db i
%assign i i+1
%endrep
db 0         ; num_parameters = 0
; Ambisonics Config
db 0         ; ambisonics_mode = 0 (MONO)
db 36        ; output_channel_count = 36
db 36        ; substream_count = 36
; channel_mapping (0 to 35)
%assign i 0
%rep 36
db i
%assign i i+1
%endrep
obu_1_end:

; Mix Presentation OBU
obu_mix_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_mix_end - obu_mix_payload ; obu_size
obu_mix_payload:
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
