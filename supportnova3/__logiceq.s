	.export f__andeq
	.export f__oreq
	.export f__xoreq

	.code

f__andeq:
	popa	2
	lda	0,0,2
	and	0,1
out:
	sta	1,0,2
	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0

f__oreq:
	popa	2
	lda	0,0,2
	com	0,0
	and	0,1
	adc	0,1
	jmp	out,1

f__xoreq:
	popa	2
	lda	0,0,2
	psha	2
	mov	1,2
	add	0,1
	andzl	0,2
	sub	2,1
	popa	2
	jmp	out,1
