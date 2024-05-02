	.export __negatel
	.code

	.setcpu 6803

	; The compiler has internal knowledge that this does not affect X
__negatel:
	std @tmp
	ldd @hireg
	coma
	comb
	std @hireg
	ldd @tmp
	coma
	comb
	addd @one
	bcc nocarry
	inc @hireg
nocarry:
	rts

	