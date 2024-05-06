;
;	(TOS) += hireg:B
;
	.setcpu 4
	.export __postincl

	.code

__postincl:
	stx	(-s)
	ldx	4(s)
	;	Low word
	lda	2(x)
	sta	(-s)
	aab
	stb	2(x)
	bl	ripple
	lda	(x)
	sta	(-s)
eq2:
	ldb	(__hireg)
	aab
	stb	(x)
	; Get the original value back
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
	ina
	bra	eq2