	.export f__shl
	.export f__shr
	.export f__shru
	.export f__shrc
	.export f__shruc
	.export f__shll
	.export f__shrl
	.export f__shrul

f__shl:
	neg	1,0		; no dec but we can negate and move in one
	lda	2,__sp,0	; so who cares
	lda	1,0,2
	dsz	__sp,0
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	done,1
shllp:
	movzl	1,1
	inc	0,0,szr
	jmp	shllp,1
done:	sta	3,__tmp,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__shru:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2
	dsz	__sp,0
shrulp:
	movzr	1,1
	inc	0,0,szr
	jmp	shrulp,1
	jmp	done,1

f__shr:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2
	dsz	__sp,0
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	done,1
	; SHR is more interesting as we need to handle sign extension
	movl#	1,1,snc	; get top bit into carry
	jmp	shrulp,1	; starts with a zero bit so use shru
shrlp:
	movor	1,1	; shift right and set high bit
	inc	0,0,szr
	jmp	shrlp,1
	jmp	done,1

f__shruc:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2
	dsz	__sp,0
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	done,1
	movs	1,1		; swap so the byte we care about is the
				; high one
shruclp:
	movzr	1,1
	inc	0,0,szr
	jmp	shruclp,1
	movs	1,1		; then swap back
	jmp	done,1

f__shrc:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2
	dsz	__sp,0
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	done,1
	movs	1,1		; same trick for signed
	movl#	1,1,snc
	jmp	shruclp,1
shrclp:
	movor	1,1
	inc	0,0,szr
	jmp	shrclp,1
	movs	1,1
	jmp	done,1

;	Same idea but 32bit wide

f__shll:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2		; low
	lda	2,-1,2		; high
	dsz	__sp,0
	dsz	__sp,0
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	store,1
llp:
	movzl	1,1		; shift left low fill 0
	movl	2,2		; and upper half
	inc	0,0,szr
	jmp	llp,1
store:
	sta	2,__hireg,0
	jmp	done,1

f__shrul:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2		; low
	lda	2,-1,2		; high
	dsz	__sp,0
	dsz	__sp,0
via_shrul:
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	store,1
rlp:
	movzr	2,2
	movr	1,1
	inc	0,0,szr
	jmp	rlp,1
	sta	2,__hireg,0
	jmp	done,1
	
f__shrl:
	neg	1,0
	lda	2,__sp,0
	lda	1,0,2		; low
	lda	2,-1,2		; high
	dsz	__sp,0
	dsz	__sp,0
	movl#	2,2,snc
	jmp	via_shrul,1
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	store,1
r1lp:
	movor	2,2
	movr	1,1
	inc	0,0,szr
	jmp	r1lp,1
	sta	2,__hireg,0
	jmp	done,1
