	.export __cast_l
	.code

__cast_l:
	bita #0x80
	beq extpv
	ldy #-1
	rts
extpv:
	ldy #0
	rts
