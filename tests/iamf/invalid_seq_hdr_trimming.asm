; IA Sequence Header OBU with invalid trimming flag
db 11111010b ; OBU Header: obu_type = 31, redundant_copy = 0, trimming = 1, extension = 0
db 8         ; obu_size = 8 bytes
db 0         ; num_samples_to_trim_at_end
db 0         ; num_samples_to_trim_at_start
db 'iamf'    ; ia_code
db 0         ; primary_profile
db 0         ; additional_profile
