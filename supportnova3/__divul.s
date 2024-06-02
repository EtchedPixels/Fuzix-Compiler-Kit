	.export f__divul
	.export f__remul
	.export f__divl
	.export f__reml
	.export f__divequl
	.export f__remequl
	.export f__diveql
	.export f__remeql

	.code

div32x32:
	sta	0,__tmp2,0
	sta	1,__tmp3,0
	popa	1	; get dividend
	popa	0
dodiv32:
	sta	3,__tmp,0
	lda	3,N32,1
	sta	3,__tmp4,0
	sub	2,2	; clear work
	sub	3,3
loop:
	movzl	1,1	; shift dividend (0,1) left
	movl	0,0
	movl	3,3	; rotate bit into work (2,3)
	movl	2,2
	psha	0
	psha	1
	lda	0,__tmp2,0
	lda	1,__tmp3,0
	subz#	0,2,snr	; work >= divisor ?
	subz#	1,3	; compare lower half
	mov#	1,1,snc ; didn't fit
	jmp	nofit,1
	subz	1,3,szc
	sub	0,2,skp
	adc	0,2
	popa	1
	inc	1,1,skp
nofit:
	popa	1
	popa	0
	dsz	__tmp4,0
	jmp	loop,1
	; Result in 0,1 remainder in 2,3
	; Save result so we can fix up AC3
	sta	2,__tmp2,0
	sta	3,__tmp3,0
	mffp	3
	jmp	@__tmp,0
N32:	.word	32

f__remul:
	sta	3,__tmp5,0
	lda	0,__hireg,0
	jsr	div32x32,1
	lda	0,__tmp2,0
	lda	1,__tmp3,0
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp5,0

f__divul:
	sta	3,__tmp5,0
	lda	0,__hireg,0
	jsr	div32x32,1
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp5,0

f__divl:
	sta	3,__tmp5,0
	lda	0,__hireg,0
	sub	2,2
	movl#	0,0,szc
	jsr	negate,1
	sta	0,__tmp2,0
	sta	1,__tmp3,0
	popa	1
	popa	0
	movl#	0,0,szc
	jsr	negate,1
	psha	2
	jsr	dodiv32,1
divout:
	popa	2
	movr#	2,2,szc
	jsr	negate,1
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp5,0

f__reml:
	sta	3,__tmp5,0
	lda	0,__hireg,0
	movl#	0,0,szc
	jsr	negate,1
	sub	2,2
	sta	0,__tmp2,0
	sta	1,__tmp3,0
	popa	1
	popa	0
	movl#	0,0,szc
	jsr	negate,1
	psha	2
	jsr	dodiv32,1
	lda	0,__tmp2,0
	lda	1,__tmp3,0
	jmp	divout,1

f__divequl:
	; Top of stack is the pointer
	popa	2
	psha	2
	psha	3
	lda	0,1,2
	psha	0
	lda	0,0,2
	psha	0
	;	Value from arg now stacked
	lda	0,__hireg,0
	;	0/1 is in the right places
	jsr	f__divul,1
	;	result is in 0/1 (and hireg:1)
	popa	3
	popa	2
	sta	0,0,2
	sta	1,1,2
	sta	3,__tmp,0
	jmp	@__tmp,0

f__remequl:
	; Top of stack is the pointer
	popa	2
	psha	2
	psha	3
	lda	0,1,2
	psha	0
	lda	0,0,2
	psha	0
	;	Value from arg now stacked
	lda	0,__hireg,0
	;	0/1 is in the right places
	jsr	f__remul,1
	;	result is in 0/1 (and hireg:1)
	popa	3
	popa	2
	sta	0,0,2
	sta	1,1,2
	sta	3,__tmp,0
	jmp	@__tmp,0

f__diveql:
	; Top of stack is the pointer
	popa	2
	psha	2
	psha	3
	lda	0,1,2
	psha	0
	lda	0,0,2
	psha	0
	;	Value from arg now stacked
	lda	0,__hireg,0
	;	0/1 is in the right places
	jsr	f__divl,1
	;	result is in 0/1 (and hireg:1)
	popa	3
	popa	2
	sta	0,0,2
	sta	1,1,2
	sta	3,__tmp,0
	jmp	@__tmp,0

f__remeql:
	; Top of stack is the pointer
	popa	2
	psha	2
	psha	3
	lda	0,1,2
	psha	0
	lda	0,0,2
	psha	0
	;	Value from arg now stacked
	lda	0,__hireg,0
	;	0/1 is in the right places
	jsr	f__reml,1
	;	result is in 0/1 (and hireg:1)
	popa	3
	popa	2
	sta	0,0,2
	sta	1,1,2
	sta	3,__tmp,0
	jmp	@__tmp,0

negate:
	sta	3,__tmp,0
	inc	2,2
	neg	1,1,snr
	neg	0,0,skp
	com	0,0
	jmp	@__tmp,0
