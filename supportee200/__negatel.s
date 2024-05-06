;
;	Negate hireg:B
;
	.export __negatel

	.setcpu 4

	.code
__negatel:
	lda	(__hireg)
	ivr	b
	iva
	inr	b
	bnz	nocarry
	ina
nocarry:
	sta	(__hireg)
	rsr
