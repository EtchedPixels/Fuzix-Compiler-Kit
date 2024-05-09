	.export __postdec
	.code

__postdec:
	; TOS is addr, AC1 value ot subtract
	sta	3,__tmp,0
	popa	2
	lda	0,0,2
	sub	1,0
	lda	1,0,2
	sta	0,0,2
	mffp	3
	jmp	@__tmp,0
