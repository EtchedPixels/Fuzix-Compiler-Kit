	.export f__pluseq
	.code

f__pluseq:
	; TOS is addr, AC1 value
	sta	3,__tmp,0
	popa	2
	lda	0,0,2
	add	0,1
	sta	1,0,2
	mffp	3
	jmp	@__tmp,0
