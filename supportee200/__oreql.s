	.setcpu	4
	.export __oreql

	.code

__oreql:
	stx	(-s)
	ldx	4(s)
	lda	2(x)
	ori	a,b
	stb	2(x)
	lda	(x)
	ldb	(__hireg)
	ori	a,b
	stb	(__hireg)
	stb	(x)
	lda	2(x)
	ldx	(s+)
	inr	s
	inr	s
	rsr
