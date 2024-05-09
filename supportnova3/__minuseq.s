	.export __minuseq
	.code

__minuseq:
	; TOS is addr, AC1 value ot subtract
	sta	3,__tmp,0
	popa	2
	lda	0,0,2
	sub	1,0
	sta	0,0,2
	mov	0,1
	mffp	3
	jmp	@__tmp,0
