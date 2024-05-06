;
;	Complement hireg:B
;
	.export __cpll

	.setcpu 4

	.code
__cpll:
	lda	(__hireg)
	ivr	b
	iva
	sta	(__hireg)
	rsr
