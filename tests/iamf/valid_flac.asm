; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31  (IA Sequence Header), redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Codec Config OBU (Valid fLaC)
obu_1_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_1_end - obu_1_start - 2 ; obu_size
db 0         ; codec_config_id = 0
db 'fLaC'    ; codec_id = 'fLaC'
db 64        ; num_samples_per_frame = 64
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
; DecoderConfig for FLAC
db 0x80      ; last_metadata_block_flag = 1, block_type = 0 (STREAMINFO)
db 0, 0, 34  ; metadata_data_block_length = 34
; STREAMINFO payload
db 0, 64     ; minimum_block_size = 64
db 0, 64     ; maximum_block_size = 64
db 0, 0, 0   ; minimum_frame_size = 0
db 0, 0, 0   ; maximum_frame_size = 0
db 0x0B, 0xB8, 0x02, 0xF0 ; sr = 48000, ch = 1 (stereo), bps = 15 (16-bit)
db 0, 0, 0, 0 ; total_samples = 0 (low 32 bits)
times 16 db 0 ; MD5 signature = 0
obu_1_end:

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

; Mix Presentation OBU (Valid)
obu_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_end - obu_start - 2 ; obu_size
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
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000 (leb128)
db 10000000b ; param_definition_mode = 1
db 0, 0      ; default_mix_gain = 0
; output_mix_gain
db 1         ; parameter_id = 1
db 0x80, 0xF7, 0x02 ; parameter_rate = 48000 (leb128)
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
