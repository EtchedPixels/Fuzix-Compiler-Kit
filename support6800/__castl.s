	.export __cast_l
	.export __cast_ul
	.code

__cast_ul:
__cast_l:
	clr @hireg
	clr @hireg+1
	tsta
	bpl extpv
	dec @hireg
	dec @hireg+1
extpv:
	rts
