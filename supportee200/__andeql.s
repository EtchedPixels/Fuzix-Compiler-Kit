	.setcpu	4
	.export __andeql

	.code

__andeql:
	stx	(-s)
	ldx	4(s)
	lda	2(x)
	nab
	stb	2(x)
	lda	(x)
	ldb	(__hireg)
	nab
	stb	(__hireg)
	stb	(x)
	lda	2(x)
	ldx	(s+)
	inr	s
	inr	s
	rsr
