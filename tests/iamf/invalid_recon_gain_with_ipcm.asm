; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Codec Config OBU: ipcm
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 1         ; codec_config_id = 1
db 'ipcm'    ; codec_id
db 64        ; num_samples_per_frame = 64
db 0, 0      ; audio_roll_distance = 0 (16 bits)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)

; Audio Element OBU: Tests "When codec_id = fLaC or ipcm, the type PARAMETER_DEFINITION_RECON_GAIN SHALL NOT be present."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 11        ; obu_size = 11 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 1         ; codec_config_id = 1 (References ipcm)
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 1         ; num_parameters = 1
; Parameter 1 (Recon Gain) -> INVALID for ipcm!
db 2         ; param_definition_type = 2 (Recon Gain)
db 0         ; parameter_id = 0
db 0         ; parameter_rate = 0
db 10000000b ; param_definition_mode = 1, reserved = 0
db 0         ; num_layers = 0

; OBU 3: Valid Mix Presentation
obu_3_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_3_end - obu_3_start - 2 ; obu_size
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
obu_3_end:

