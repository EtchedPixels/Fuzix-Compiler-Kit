	.setcpu	4
	.export __xorl

	.code

__xorl:
	lda	4(s)
	ore	a,b
	stb	4(s)
	lda	2(s)
	ldb	(__hireg)
	ore	a,b
	stb	(__hireg)
	ldb	4(s)
	lda	4
	add	a,s
	rsr
