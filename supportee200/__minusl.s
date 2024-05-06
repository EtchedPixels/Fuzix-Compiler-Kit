;
;	TOS - B:hireg
;
	.setcpu	4
	.export __minusl

	.code

__minusl:
	lda	4(s)
	sab
	stb	4(s)		; save it for a moment
	bl	ripple
	lda	2(s)
minus2:
	ldb	(__hireg)
	sab
	stb	(__hireg)
	ldb	4(s)
	lda	4
	add	a,s
	rsr
ripple:
	lda	2(s)
	dca
	bra	minus2
