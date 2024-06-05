	.export f__andeq
	.export f__oreq
	.export f__xoreq

	.code

f__andeq:
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	lda	0,0,2
	and	0,1
out:
	sta	1,0,2
	sta	3,__tmp,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__oreq:
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	lda	0,0,2
	com	0,0
	and	0,1
	adc	0,1
	jmp	out,1

f__xoreq:
	lda	2,__sp,0
	lda	2,0,2
	lda	0,0,2
	mov	1,2
	add	0,1
	andzl	0,2
	sub	2,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	jmp	out,1
