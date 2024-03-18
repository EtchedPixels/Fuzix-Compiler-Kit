	.export __negatel
	.code
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

	