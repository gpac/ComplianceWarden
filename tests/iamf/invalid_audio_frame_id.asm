; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile

; OBU 1: Audio Frame with invalid ID
db 00101000b ; OBU Header: obu_type = 5 (Audio Frame), redundant_copy = 0, trimming = 0, extension = 0
db 2         ; obu_size = 2 bytes
db 10        ; explicit_audio_substream_id = 10 -> INVALID (must be > 17)
db 0         ; dummy audio frame byte
