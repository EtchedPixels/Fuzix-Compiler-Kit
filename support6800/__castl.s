	.export __cast_l
	.export __cast_ul
	.code

__cast_ul:
__cast_l:
	clr @hireg
	clr @hireg+1
	bita #0x80
	beq extpv
	dec @hireg
	dec @hireg+1
extpv:
	rts
