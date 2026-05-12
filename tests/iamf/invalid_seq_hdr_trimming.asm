; IA Sequence Header OBU with invalid trimming flag
db 11111010b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 1, extension = 0
db 8         ; obu_size = 8 bytes
db 0         ; num_samples_to_trim_at_end
db 0         ; num_samples_to_trim_at_start
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Codec Config OBU (Valid ipcm)
obu_codec_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_codec_end - obu_codec_start - 2 ; obu_size
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 64        ; num_samples_per_frame = 64
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)
obu_codec_end:

; OBU 2: Valid Audio Element
obu_ae_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_ae_end - obu_ae_start - 2 ; obu_size
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
obu_ae_end:

; OBU 3: Valid Mix Presentation
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
obu_mix_end:
