;
;	(TOS) += hireg:B
;
	.setcpu 4
	.export __pluseql

	.code

__pluseql:
	stx	(-s)
	ldx	4(s)
	;	Low word
	lda	2(x)
	aab
	stb	2(x)
	bl	ripple
	lda	(x)
eq2:
	ldb	(__hireg)
	aab
	stb	(x)
	stb	(__hireg)
	ldb	2(x)
	ldx	(s+)
	inr	s
	inr	s
	rsr
ripple:
	lda	(x)
	ina
	bra	eq2