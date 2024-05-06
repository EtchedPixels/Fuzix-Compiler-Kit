;
;	16bit unsigned div/mod
;
	.setcpu 4
	.export __divu
	.export __remu

	.code

__divu:
	lda	2(s)
	jsr	__div16x16
	xab
	lda	(s+)	; discard a word
	rsr

__remu:
	lda	2(s)
	jsr	__div16x16
	lda	(s+)
	rsr
