%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

aaaa_start:
dd BE(aaaa_end - aaaa_start)
db "aaaa"
aaaa_end:
