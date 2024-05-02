	.export __negatel
	.code

	; The compiler has internal knowledge that this does not affect X
__negatel:
	xgdy
	coma
	comb
	xgdy
	coma
	comb
	addd @one
	bcc nocarry
	iny
nocarry:
	rts

	