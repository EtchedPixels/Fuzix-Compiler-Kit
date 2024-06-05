	.export f__minuseql

f__minuseql:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2		; get ptr but don't remove
	mov	1,3
	lda	1,1,2
	lda	0,0,2
	lda	2,__hireg,0
	; Now doing 3,2 - 1,0
	subz	3,1,szc
	sub	2,0,skp
	adc	2,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	0,0,2
	sta	1,1,2
	sta	0,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp,0
