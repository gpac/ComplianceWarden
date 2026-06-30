; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile = 0 (Simple)
db 0         ; additional_profile

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

; Audio Element OBU
; INVALID: The sum of channels across layers (9 + 9 = 18) exceeds the Simple Profile limit of 16.
; We use dummy values (9 substreams each) for Mono and Stereo layouts to ensure
; neither layer alone exceeds 16, thus specifically testing the summation logic.
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 30        ; obu_size = 30 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED)
db 0         ; codec_config_id = 0
db 18        ; num_substreams = 18
db 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 ; audio_substream_ids
db 0         ; num_parameters = 0
db 01000000b ; num_layers = 2
; Layer 1
db 0         ; loudspeaker_layout = 0 (Mono)
db 9         ; substream_count = 9 (INVALID: Mono layout expects 1)
db 0         ; coupled_substream_count = 0
; Layer 2
db 1         ; loudspeaker_layout = 1 (Stereo)
db 9         ; substream_count = 9 (INVALID: Stereo layout expects different count)
db 0         ; coupled_substream_count = 0
