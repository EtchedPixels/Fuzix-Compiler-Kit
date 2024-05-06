	.setcpu	4
	.export __xoreql

	.code

__xoreql:
	stx	(-s)
	ldx	4(s)
	lda	2(x)
	ore	a,b
	stb	2(x)
	lda	(x)
	ldb	(__hireg)
	ore	a,b
	stb	(__hireg)
	stb	(x)
	lda	2(x)
	ldx	(s+)
	inr	s
	inr	s
	rsr
