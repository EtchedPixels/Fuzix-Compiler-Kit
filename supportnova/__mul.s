	.export f__mul

	.code

f__mul:
	lda	2,__sp,0
	lda	2,0,2		; get argument off stack
	dsz	__sp,0
	subc	0,0
	sta	3,__tmp,0
	lda	3,N16,1
loop:	movzl	0,0		; shift result left
	movzl	1,1,szc		; shift input left and bit into carry
	add	2,0		; if it was set add
	inc	3,3,szr
	jmp	loop,1
	mov	0,1
	lda	3,__fp,0
	jmp	@__tmp,0
N16:	.word	-16

f__muleq:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,@0,2		; argument off stack (ptr is on stack)
	jsr	f__mul,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	1,0,2		; stuff it back
	lda	3,__fp,0
	jmp	@__tmp2,0
