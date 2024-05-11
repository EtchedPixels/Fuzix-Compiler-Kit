;
;	Unsigned 16bit divide
;
	.export f__divu
	.export f__remu
	.export f__divequ
	.export f__remequ

	.code
f__divu:
	popa	2
	sub	0,0
	sta	3,__tmp,0
;	sub#	2,0,szc
;	jmp	__diverror
	lda	3,N16,1
	movzl	1,1
loop:	movl	0,0
	sub#	2,0,szc
	sub	2,0
	movl	1,1
	inc	3,3,szr
	jmp	loop,1
	mffp	3
	jmp	@__tmp
N16:	.word	0

f__remu:
	psha	3
	jsr	f__divu,1
	popa	3
	sta	3,__tmp,0
	mov	0,1
	mffp	3
	jmp	@__tmp

f__divequ:
	popa	2
	psha	3
	psha	2
	lda	2,0,2
	jsr	f__divu,1
	popa	2
	popa	3
	sta	3,__tmp,0
	sta	1,0,2
	mffp	3
	jmp	@__tmp

f__remequ:
	popa	2
	psha	3
	psha	2
	lda	2,0,2
	jsr	f__divu,1
	popa	2
	popa	3
	sta	3,__tmp,0
	mov	0,1
	sta	1,0,2
	mffp	3
	jmp	@__tmp
