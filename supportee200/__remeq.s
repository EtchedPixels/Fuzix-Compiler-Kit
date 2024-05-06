	.setcpu 4
	.export __remeq
	.export __remeqc

	.code

__remeq:
	lda	2(s)
	lda	(a)
	sta	(s+)
	jsr	__rem
	; Removed the word we pushed. Result is in B
	lda	2(s)
	stb	(a)
	inr	s
	inr	s
	rsr

__remeqc:
	lda	2(s)
	ldab	(a)
	sta	(s+)
	jsr	__rem
	lda	2(s)
	stb	(a)
	inr	s
	inr	s
	rsr
