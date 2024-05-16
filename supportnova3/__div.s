;
;	Unsigned 16bit divide
;
	.export f__divu
	.export f__remu
	.export f__divequ
	.export f__remequ
	.export f__div
	.export f__rem

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
	jmp	@__tmp,0
N16:	.word	0

f__remu:
	psha	3
	jsr	f__divu,1
	popa	3
	sta	3,__tmp,0
	mov	0,1
	mffp	3
	jmp	@__tmp,0

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
	jmp	@__tmp,0

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
	jmp	@__tmp,0

f__div:
	popa	2	; other arg into 2
	psha	3	; save return
	sub	0,0	; 0 will track signs
	movl#	1,1,szc
	jmp	__neg1,1
n1:
	movl#	2,2,szc
	jmp	neg2,1
n2:
	psha	0
	psha	2
	jsr	f__divu,1
done:
	; Now have result to sign correct
	movr	0,0,szc	; sign change if low bit set
	neg	1,1
	popa	3
	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0
neg1:
	inc	0,0
	neg	1,1
	jmp	n2,1
neg2:
	inc	0,0
	neg	2,2
	jmp	n2,1


f__rem:
	popa	2
	psha	3
	sub	0,0
	movl#	2,2,szc
	neg	2,2
	movl#	1,1,szc
	jmp	neg3,1
n3:
	psha	2
	jsr	f__divu,1
	mov	0,1
	jmp	done,1
neg3:
	inc	0,0
	neg	1,1
	jmp	n3,1

f__diveq:
	popa	2
	psha	3
	psha	2
	lda	2,0,2
	jsr	f__div,1
	popa	2
	popa	3
	sta	3,__tmp,0
	sta	1,0,2
	mffp	3
	jmp	@__tmp,0

f__remeq:
	popa	2
	psha	3
	psha	2
	lda	2,0,2
	jsr	f__rem,1
	popa	2
	popa	3
	sta	3,__tmp,0
	sta	1,0,2
	mffp	3
	jmp	@__tmp,0
