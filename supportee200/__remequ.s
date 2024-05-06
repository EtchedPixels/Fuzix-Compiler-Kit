	.setcpu 4
	.export __remequ
	.export __remequc

	.code

__remequ:
	lda	2(s)
	lda	(a)
	sta	(s+)
	jsr	__remu
	; Removed the word we pushed. Result is in B
	lda	2(s)
	stb	(a)
	inr	s
	inr	s
	rsr

__remequc:
	lda	2(s)
	ldab	(a)
	sta	(s+)
	jsr	__remu
	lda	2(s)
	stb	(a)
	inr	s
	inr	s
	rsr
