;
;	CC unsigned
;
	.export f__condeq
	.export f__condne
	.export f__condltequ
	.export f__condgtu
	.export f__condgtequ
	.export f__condltu
;
;	Signed compare
;
;	Move the numbers up by 128 signed (toggle top bit)
;	Do unsigned compare now they are in 0-255 range
;
	.export f__condlteq
	.export f__condgt
	.export f__condgteq
	.export f__condlt

	.code

f__condeq:
	lda	0,0,3
	sub#	0,1,snr		; skip if false
	subzl	1,1,skp		; set one and skip
	sub	1,1		; set 0 otherwise
condout:
	inc	3,3
	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0

f__condne:
	lda	0,0,3
	sub	0,1,szr
	subzl	1,1,skp
	jmp	condout,1

f__condltequ:
	lda	0,0,3
	subz#	1,0,szc
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
	adcz#	1,0,snc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condltu:
	lda	0,0,3
	adcz#	1,0,szc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condlteq:
	lda	0,0,3
	movl	0,0		; shift top bit into carry
	movcr	0,0		; complement and put back
	movl	1,1		; shift top bit into carry
	movcr	1,1		; complement and put back
	subz#	1,0,szc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condgt:
	lda	0,0,3
	movl	0,0		; shift top bit into carry
	movcr	0,0		; complement and put back
	movl	1,1		; shift top bit into carry
	movcr	1,1		; complement and put back
	subz#	1,0,snc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condgteq:
	lda	0,0,3
	movl	0,0		; shift top bit into carry
	movcr	0,0		; complement and put back
	movl	1,1		; shift top bit into carry
	movcr	1,1		; complement and put back
	adcz#	1,0,snc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1

f__condlt:
	lda	0,0,3
	movl	0,0		; shift top bit into carry
	movcr	0,0		; complement and put back
	movl	1,1		; shift top bit into carry
	movcr	1,1		; complement and put back
	adcz#	1,0,szc
	subzl	1,1,skp
	sub	1,1
	jmp	condout,1
