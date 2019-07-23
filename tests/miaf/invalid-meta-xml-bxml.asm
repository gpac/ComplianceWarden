%define BE(a) ( ((((a)>>24)&0xFF) << 0) + ((((a)>>16)&0xFF) << 8) + ((((a)>>8)&0xFF) << 16)  + ((((a)>>0)&0xFF) << 24) )

ftyp_start:
dd BE(ftyp_end - ftyp_start)
db "ftyp"

db "isom"
dd BE(0x200)
db "mif1", "miaf"

ftyp_end:

meta_start:
dd BE(meta_end - meta_start)
db "meta"
dd BE(0)

dinf_start:
dd BE(dinf_end - dinf_start)
db "dinf"
dinf_end:

pitm_start:
dd BE(pitm_end - pitm_start)
db "pitm"
pitm_end:

iinf_start:
dd BE(iinf_end - iinf_start)
db "iinf"
dd BE(0)
iinf_end:

xml_start:
dd BE(xml_end - xml_start)
db "xml "
xml_end:

bxml_start:
dd BE(bxml_end - bxml_start)
db "bxml"
bxml_end:

iprp_start:
dd BE(iprp_end - iprp_start)
db "iprp"
ipco_start:
dd BE(ipco_end - ipco_start)
db "ipco"
ispe_start:
dd BE(ispe_end - ispe_start)
db "ispe"
ispe_end:
ipco_end:
iprp_end:

meta_end:

; vim: syntax=nasm
