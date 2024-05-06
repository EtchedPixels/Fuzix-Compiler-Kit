	.setcpu	4
	.export __bandl

	.code

__bandl:
	lda	4(s)
	nab
	stb	4(s)
	lda	2(s)
	ldb	(__hireg)
	nab
	stb	(__hireg)
	ldb	4(s)
	lda	4
	add	a,s
	rsr
