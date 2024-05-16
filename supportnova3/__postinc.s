	.export f__postinc
	.export f__postincl
	.code

f__postinc:
	; TOS is addr, AC1 value
	sta	3,__tmp,0
	popa	2
	lda	0,0,2
	add	0,1
	sta	1,0,2
	mov	0,1
	mffp	3
	jmp	@__tmp,0

f__postincl:
	sta	3,__tmp,0
	popa	2
	psha	2
	lda	3,1,2
	lda	2,0,2
	lda	0,__hireg,0
	sta	2,__hireg,0
	add	2,0
	add	3,1
	popa	2
	sta	0,0,2
	sta	1,1,2
	mov	3,1
	mffp	3
	jmp	@__tmp,0

	