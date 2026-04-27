; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; Audio Element OBU 1
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 7         ; obu_size = 7 bytes
db 1         ; audio_element_id = 1
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 1         ; audio_substream_id = 1
db 0         ; num_parameters = 0
db 0         ; num_layers = 0

; Audio Element OBU 2: Tests "audio_element_id SHALL be unique within an IA Sequence."
db 00001000b ; OBU Header: obu_type = 1 (Audio Element)
db 7         ; obu_size = 7 bytes
db 1         ; audio_element_id = 1 -> DUPLICATE!
db 0         ; audio_element_type = 0 (CHANNEL_BASED), reserved = 0
db 0         ; codec_config_id = 0
db 1         ; num_substreams = 1
db 2         ; audio_substream_id = 2
db 0         ; num_parameters = 0
db 0         ; num_layers = 0
