	.export f__minuseql

f__minuseql:
	sta	3,__tmp,0
	popa	2
	psha	2
	mov	1,3
	lda	1,1,2
	lda	0,0,2
	lda	2,__hireg,0
	; Now doing 3,2 - 1,0
	subz	3,1,szc
	sub	2,0,skp
	adc	2,0
	popa	2
	sta	0,0,2
	sta	1,1,2
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp,0
