; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: mp4a with invalid fields in DecoderConfigDescriptor and AudioSpecificConfig
obu_1_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_1_end - obu_1_start - 2 ; obu_size
db 1         ; codec_config_id = 1
db 'mp4a'    ; codec_id = 'mp4a'
db 100       ; num_samples_per_frame = 100
db 0xFF      ; audio_roll_distance = -1
db 0xFF      ; audio_roll_distance (low byte)
; DecoderConfig for AAC
db 0x05      ; decoder_config_descriptor_tag = 5 -> INVALID (should be 4)
db 19        ; decoder_config_descriptor_length = 19
db 0x41      ; objectTypeIndication = 0x41 -> INVALID (should be 0x40)
; streamType = 6 (6 bits), upstream = 1 (1 bit), reserved = 1 (1 bit)
; binary: 000110 (6) | 1 (upstream) | 1 (reserved) -> 00011011b = 0x1b -> INVALID
db 0x1b
db 0, 0, 0   ; bufferSizeDB = 0
db 0, 0, 0, 0 ; maxBitrate = 0
db 0, 0, 0, 0 ; avgBitrate = 0
db 0x06      ; decoder_specific_info_tag = 6 -> INVALID (should be 5)
db 4         ; decoder_specific_info_length = 4
; AudioSpecificConfig:
; audioObjectType = 3 (5 bits) -> INVALID (should be 2)
; samplingFrequencyIndex = 3 (4 bits) (48000 Hz)
; channelConfiguration = 3 (4 bits) -> INVALID (should be 2)
; frameLengthFlag = 1 (1 bit) -> INVALID (should be 0)
; dependsOnCoreCoder = 1 (1 bit) -> INVALID (should be 0)
;   since dependsOnCoreCoder = 1, coreCoderDelay = 0 (14 bits)
; extensionFlag = 1 (1 bit) -> INVALID (should be 0)
; Concat bits: 00011 + 0011 + 0011 + 1 + 1 + 00000000000000 + 1
; Packed: 0x19, 0x9e, 0x00, 0x04
db 0x19, 0x9e, 0x00, 0x04
obu_1_end:

; OBU 2: Valid Codec Config (ipcm) to allow reference
obu_2_start:
db 00000000b ; OBU Header: obu_type = 0 (Codec Config)
db obu_2_end - obu_2_start - 2 ; obu_size
db 4         ; codec_config_id = 4
db 'ipcm'    ; codec_id = 'ipcm'
db 64        ; num_samples_per_frame = 64
db 0         ; audio_roll_distance (high byte)
db 0         ; audio_roll_distance (low byte)
db 1         ; sample_format_flags = 1 (little-endian)
db 16        ; sample_size = 16
db 0, 0, 0xBB, 0x80 ; sample_rate = 48000 (big-endian)
obu_2_end:

; OBU 3: Valid Audio Element (referencing valid OBU 2)
obu_3_start:
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db obu_3_end - obu_3_start - 2 ; obu_size
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 4         ; codec_config_id = 4
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 00100000b ; num_layers = 1
db 0         ; loudspeaker_layout = 0 (Mono)
db 1         ; substream_count = 1
db 0         ; coupled_substream_count = 0
obu_3_end:

; OBU 4: Valid Mix Presentation
obu_4_start:
db 00010000b ; OBU Header: obu_type = 2 (Mix Presentation)
db obu_4_end - obu_4_start - 2 ; obu_size
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
obu_4_end:
