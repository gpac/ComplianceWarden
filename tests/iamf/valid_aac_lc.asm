; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Valid Codec Config (mp4a)
obu_1_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; codec_config_id = 1
db 'mp4a'    ; codec_id = 'mp4a'
db 0x80, 0x08 ; num_samples_per_frame = 1024 (leb128)
db 0xFF      ; audio_roll_distance = -1
db 0xFF      ; audio_roll_distance (low byte)
; DecoderConfig for AAC
db 0x04      ; decoder_config_descriptor_tag = 4
db 17        ; decoder_config_descriptor_length = 17
db 0x40      ; objectTypeIndication = 0x40
db 0x15      ; streamType = 5 (6 bits), upstream = 0 (1 bit), reserved = 1 (1 bit)
db 0, 0, 0   ; bufferSizeDB = 0
db 0, 0, 0, 0 ; maxBitrate = 0
db 0, 0, 0, 0 ; avgBitrate = 0
db 0x05      ; decoder_specific_info_tag = 5
db 2         ; decoder_specific_info_length = 2
; AudioSpecificConfig: AOT=2, freq_index=3 (48000 Hz), channel_config=2, flags=0
; Packed: 0x11, 0x90
db 0x11, 0x90
obu_1_end:

; OBU 2: Valid Audio Element (referencing OBU 1)
obu_2_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_2_end - obu_2_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 1         ; codec_config_id = 1
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1
db 0         ; loudspeaker_layout = 0 (Mono)
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_2_end:

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
db 10000000b ; layout_type = 2 (LOUDSPEAKERS_SS_CONVENTION), sound_system = 0 (Stereo)
; Loudness
db 0         ; info_type = 0
db 0, 0      ; integrated_loudness = 0
db 0, 0      ; digital_peak = 0
obu_3_end:
