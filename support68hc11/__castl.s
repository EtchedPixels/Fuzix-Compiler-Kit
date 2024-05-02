	.export __cast_l
	.export __cast_ul
	.code

__cast_ul:
__cast_l:
	bita #0x80
	beq extpv
	ldy #-1
	rts
extpv:
	ldy #0
	rts
