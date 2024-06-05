	.export f__minuseq
	.code

f__minuseq:
	; TOS is addr, AC1 value ot subtract
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	lda	0,0,2
	sub	1,0
	sta	0,0,2
	mov	0,1
	lda	3,__fp,0
	jmp	@__tmp,0
