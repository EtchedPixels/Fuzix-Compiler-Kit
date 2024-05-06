	.setcpu 4
	.export __muleq
	.export __mulequ
	.export __muleqc
	.export __mulequc

	.code

__muleq:
__mulequ:
	lda	2(s)
	lda	(a)
	sta	(s+)
	jsr	__mul
	; Removed the word we pushed. Result is in B
	lda	2(s)
	stb	(a)
	rsr

__muleqc:
__mulequc:
	lda	2(s)
	ldab	(a)
	sta	(s+)
	jsr	__mul
	lda	2(s)
	stb	(a)
	rsr
