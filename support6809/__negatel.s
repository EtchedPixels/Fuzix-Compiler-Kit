	.export __negatel
	.code

	; The compiler has internal knowledge that this does not affect X
__negatel:
	exg d,y
	coma
	comb
	exg d,y
	coma
	comb
	addd @one
	bcc nocarry
	leay 1,y
nocarry:
	rts

	