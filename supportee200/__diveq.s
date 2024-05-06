	.setcpu 4
	.export __diveq
	.export __diveqc

	.code

__diveq:
	lda	2(s)
	lda	(a)
	sta	(s+)
	jsr	__div
	; Removed the word we pushed. Result is in B
	lda	2(s)
	stb	(a)
	inr	s
	inr	s
	rsr

__diveqc:
	lda	2(s)
	ldab	(a)
	sta	(s+)
	jsr	__div
	lda	2(s)
	stb	(a)
	inr	s
	inr	s
	rsr
