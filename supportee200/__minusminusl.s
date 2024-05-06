;
;	(TOS) -= hireg:B
;
	.setcpu 4
	.export __postdecl

	.code

__postdecl:
	stx	(-s)
	ldx	4(s)
	;	Low word
	lda	2(x)
	sta	(-s)
	sab
	stb	2(x)
	bl	ripple
	lda	(x)
	sta	(-s)
eq2:
	ldb	(__hireg)
	sab
	stb	(x)
	ldb	2(x)
	lda	(s+)
	sta	(__hireg)
	ldb	(s+)
	ldx	(s+)
	inr	s
	inr	s
	rsr
ripple:
	lda	(x)
	sta	(-s)
	dca
	bra	eq2
