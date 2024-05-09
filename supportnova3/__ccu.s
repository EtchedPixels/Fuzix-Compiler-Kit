;
;	CC unsigned
;
	.export f__condeq
	.export f__condne
	.export f__condltequ
	.export f__condgtu
	.export f__condgtequ
	.export f__condltu

	.code

f__condeq:
	lda	0,0,3
	sub#	0,1,snr		; skip if false
	subzl	1,1,skp		; set one and skip
	sub	1,1		; set 0 otherwise
condout:
	inc	3
	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0

f__condne:
	lda	0,0,3
	sub#	0,1,szr
	subzl	1,1
	jmp	condout,1

f__condltequ:
	lda	0,0,3
	subz#	0,1,snc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condgtu:
	lda	0,0,3
	subz#	1,0,snc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condgtequ:
	lda	0,0,3
	adcz#	0,1,snc
	subz	1,1,skp
	sub	1,1
	jmp	condout,1

f__condlt:
	lda	0,0,3
	adcz#	1,0,snc
	subz	1,1,skp
	sub	1,1
	jmp	condout,1

