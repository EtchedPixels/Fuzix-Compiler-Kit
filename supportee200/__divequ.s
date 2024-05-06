	.setcpu 4
	.export __divequ
	.export __divequc

	.code

__divequ:
	lda	2(s)
	lda	(a)
	sta	(s+)
	jsr	__divu
	; Removed the word we pushed. Result is in B
	lda	2(s)
	stb	(a)
	rsr

__divequc:
	lda	2(s)
	ldab	(a)
	sta	(s+)
	jsr	__divu
	lda	2(s)
	stb	(a)
	rsr
