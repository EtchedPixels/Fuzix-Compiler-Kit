	.export f__pluseq
	.export f__cpluseq
	.code

f__cpluseq:
	inc	3,3
	mov	1,2
	lda	1,-1,3
peq:
	sta	3,__tmp,0
	lda	0,0,2
	addz	0,1
	sta	1,0,2
	lda	3,__fp,0
	jmp	@__tmp,0

f__pluseq:
	; TOS is addr, AC1 value
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	jmp	peq,1
