; IA Sequence Header OBU
db 11111000b ; OBU Header: obu_type = 31 (IA Sequence Header), redundant_copy = 0, trimming = 0, extension = 0
db 6         ; obu_size = 6 bytes
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile
