; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Valid ipcm
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 14        ; obu_size = 14 bytes
db 0         ; codec_config_id = 0
db 'ipcm'    ; codec_id = 'ipcm'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)

; OBU 2: Duplicate codec_config_id = 0
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 0         ; codec_config_id = 0 -> INVALID (duplicate)
db 'mp4a'    ; codec_id = 'mp4a'
db 100       ; num_samples_per_frame = 100
db 0xFF      ; audio_roll_distance = -1
db 0xFF      ; audio_roll_distance (low byte)

; OBU 3: Invalid codec_id
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 1         ; codec_config_id = 1
db 'xxxx'    ; codec_id = 'xxxx' -> INVALID
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance
db 0         ; audio_roll_distance (low byte)

; OBU 4: mp4a with invalid roll distance (0 instead of -1)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 8         ; obu_size = 8 bytes
db 2         ; codec_config_id = 2
db 'mp4a'    ; codec_id = 'mp4a'
db 100       ; num_samples_per_frame = 100
db 0         ; audio_roll_distance = 0 -> INVALID (mp4a requires -1)
db 0         ; audio_roll_distance (low byte)

; OBU 5: fLaC with invalid roll distance (-1 instead of 0)
obu_5_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_5_end - obu_5_start - 2 ; obu_size
db 3         ; codec_config_id = 3
db 'fLaC'    ; codec_id = 'fLaC'
db 100       ; num_samples_per_frame = 100
db 0xFF      ; audio_roll_distance = -1 -> INVALID (fLaC requires 0)
db 0xFF      ; audio_roll_distance (low byte)
; DecoderConfig for FLAC
db 0x80      ; last_metadata_block_flag = 1, block_type = 0 (STREAMINFO)
db 0, 0, 34  ; metadata_data_block_length = 34
; STREAMINFO payload
db 0, 100    ; minimum_block_size = 100
db 0, 100    ; maximum_block_size = 100
db 0, 0, 0   ; minimum_frame_size = 0
db 0, 0, 0   ; maximum_frame_size = 0
db 0x0B, 0xB8, 0x02, 0xF0 ; sr = 48000, ch = 1 (stereo), bps = 15 (16-bit)
db 0, 0, 0, 0 ; total_samples = 0 (low 32 bits)
times 16 db 0 ; MD5 signature = 0
obu_5_end:

; OBU 6: Opus with invalid roll distance (0 instead of -4)
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db 20        ; obu_size = 20 bytes
db 4         ; codec_config_id = 4
db 'Opus'    ; codec_id = 'Opus'
db 0xC0, 0x07 ; num_samples_per_frame = 960
db 0         ; audio_roll_distance = 0 -> INVALID (Opus requires -4)
db 0
; DecoderConfig for Opus
db 1         ; version = 1
db 2         ; output_channel_count = 2
db 0, 0      ; pre_skip = 0
db 0, 0, 0xBB, 0x80 ; input_sample_rate = 48000
db 0, 0      ; output_gain = 0
db 0         ; channel_mapping_family = 0

; OBU 7: Valid Audio Element
obu_7_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_7_end - obu_7_start - 2 ; obu_size
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
obu_7_end:

; OBU 8: Valid Mix Presentation
obu_8_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_8_end - obu_8_start - 2 ; obu_size
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
obu_8_end:

