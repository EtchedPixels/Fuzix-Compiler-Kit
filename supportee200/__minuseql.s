;
;	(TOS) -= hireg:B
;
	.setcpu 4
	.export __minuseql

	.code

__minuseql:
	stx	(-s)
	ldx	4(s)
	;	Low word
	lda	2(x)
	sab
	stb	2(x)
	bl	ripple
	lda	(x)
eq2:
	ldb	(__hireg)
	sab
	stb	(x)
	stb	(__hireg)
	ldb	2(x)
	ldx	(s+)
	inr	s
	inr	s
	rsr
ripple:
	lda	(x)
	dca
	bra	eq2
