	.setcpu	4
	.export __orl

	.code

__orl:
	lda	4(s)
	ori	a,b
	stb	4(s)
	lda	2(s)
	ldb	(__hireg)
	ori	a,b
	stb	(__hireg)
	ldb	4(s)
	lda	4
	add	a,s
	rsr
