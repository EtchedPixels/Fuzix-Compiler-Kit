;
;	Unsigned 16bit divide
;
	.export f__divu
	.export f__remu
	.export f__divequ
	.export f__remequ
	.export f__div
	.export f__rem
	.export f__diveq
	.export f__remeq

	.code

f__divu:
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sub	0,0		; working value in register 0
	sta	3,__tmp,0	; save return so we can use AC3
;	sub#	2,0,szc
;	jmp	__diverror
	lda	3,N16,1		; AC3 is count for 16 runs

loop:	movzl	2,2		; shift dividend left
	movl	0,0		; roll into working
	subz#	1,0,snc		; if can subtract
	jmp	next,1		; can't subtract
	sub	1,0		; subtract (will set L)
	inc	2,2		; set low bit in dividend
next:
	inc	3,3,szr		; are we there yet ?
	jmp	loop,1		; next iteration
	lda	3,__fp,0
	;	Result is in AC2, remainder is in AC0
	mov	2,1		; result into AC1
	jmp	@__tmp,0
N16:	.word	-16


f__remu:
	sta	3,__tmp2,0
	jsr	f__divu,1
	mov	0,1
	lda	3,__fp,0
	jmp	@__tmp2,0

f__divequ:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,@0,2
	jsr	f__divu,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	1,0,2
	lda	3,__fp,0
	jmp	@__tmp2,0

f__remequ:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,@0,2
	jsr	f__divu,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	mov	0,1
	sta	1,0,2
	lda	3,__fp,0
	jmp	@__tmp2,0

f__div:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sub	0,0	; 0 will track signs
	movl#	1,1,szc
	jmp	neg1,1
n1:
	movl#	2,2,szc
	jmp	neg2,1
n2:
	sta	0,@__spm0
	sta	2,@__spm0
	jsr	f__divu,1
done:
	lda	3,__sp,0
	lda	0,0,3
	dsz	__sp,0
	; Now have result to sign correct
	movr	0,0,szc	; sign change if low bit set
	neg	1,1
	lda	3,__fp,0
	jmp	@__tmp2,0
neg1:
	inc	0,0
	neg	1,1
	jmp	n2,1
neg2:
	inc	0,0
	neg	2,2
	jmp	n2,1


f__rem:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sub	0,0
	movl#	1,1,szc
	neg	1,1	; get right side positive
	movl#	2,2,szc
	jmp	neg3,1	; if left side is negative
n3:
	sta	0,@__sp,0	; do the divide
	sta	2,@__sp,0
	jsr	f__divu,1
	mov	0,1	; remainder
	jmp	done,1
neg3:
	inc	0,0	; remember to invert result
	neg	2,2	; negate value
	jmp	n3,1	; into divide

f__diveq:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,@0,2
	jsr	f__div,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	1,0,2
	lda	3,__fp,0
	jmp	@__tmp2,0

f__remeq:
	sta	3,__tmp2,0
	lda	2,__sp,0
	lda	2,@0,2
	jsr	f__rem,1
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	1,0,2
	lda	3,__fp,0
	jmp	@__tmp2,0
