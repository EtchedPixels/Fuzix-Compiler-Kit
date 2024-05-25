	.export f__mul

	.code

f__mul:
	popa	2		; get argument off stack
	subc	0,0
	sta	3,__tmp,0
	lda	3,N16,1
loop:	movzl	0,0		; shift result left
	movzl	1,1,szc		; shift input left and bit into carry
	add	2,0		; if it was set add
	inc	3,3,szr
	jmp	loop,1
	mov	0,1
	mffp	3
	jmp	@__tmp,0
N16:	.word	-16

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
