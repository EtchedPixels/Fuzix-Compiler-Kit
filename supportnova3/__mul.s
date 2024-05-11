	.export f__mul

	.code

f__mul:
	popa	2		; get argument off stack
	subc	0,0
	sta	3,__tmp,0
	lda	3,N16,1
loop:	movr	1,1,snc
	movr	0,0,skp
	addzr	2,0
	inc	3,3,szr
	jmp	loop,1
	mffp	3
	jmp	@__tmp,0
N16:	.word	16

f__muleq:
	popa	2
	psha	3
	psha	2
	lda	2,0,2		; argument off stack
	jsr	f__mul,1
	popa	2
	sta	1,0,2		; stuff it back
	popa	3
	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0
