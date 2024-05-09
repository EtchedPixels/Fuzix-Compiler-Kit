;
;	Unsigned 16bit divide
;
	.export __divu
	.code
__divu:
	sub	0,0
	sta	3,__tmp,0
	sub#	2,0,szc
	jmp	__diverror
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

	