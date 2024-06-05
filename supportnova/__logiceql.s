	.export f__andeql
	.export f__oreql
	.export f__xoreql
	.export f__andl
	.export f__orl
	.export f__xorl

	.code

f__andeql:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	lda	0,0,2
	lda	3,__hireg,0
	and	0,3
	lda	0,1,2
	and	0,1
out:
	sta	3,__hireg,0
	sta	3,0,2
	sta	1,1,2
	lda	3,__fp,0
	jmp	@__tmp,0

f__oreql:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	lda	0,0,2
	lda	3,__hireg,0
	com	0,0
	and	0,3
	adc	0,3
	lda	0,1,2
	com	0,0
	and	0,1
	adc	0,1
	jmp	out,1

;
;	Gets messy as we need a scratch register for xol
;
f__xoreql:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	0,@0,2
	lda	3,__hireg,0
	mov	3,2
	andzl	0,2
	add	0,3
	sub	2,3
	lda	2,__sp,0
	lda	2,0,2
	lda	0,1,2
	mov	1,2
	andzl	0,2
	add	0,1
	sub	2,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	jmp	out,1

f__andl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	3,-1,2
	lda	2,0,2
	dsz	sp
	dsz	sp
	lda	0,__hireg,0
	and	2,0
	and	3,1
	sta	0,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__orl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	3,-1,2
	lda	2,0,2
	dsz	sp
	dsz	sp
	lda	0,__hireg,0
	com	2,2
	and	2,0
	adc	2,0
	com	3,3
	and	3,1
	adc	3,1
	sta	0,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__xorl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0		; high
	lda	0,__hireg,0
	mov	0,3
	andzl	2,3
	add	2,0
	sub	3,0
	sta	0,__hireg,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0		; low
	mov	1,3
	andzl	2,3
	add	2,1
	sub	3,1
	lda	3,__fp,0
	jmp	@__tmp,0
