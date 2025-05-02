	.export f__shl
	.export f__shr
	.export f__shru
	.export f__shll
	.export f__shrl
	.export f__shrul

f__shl:
	neg	1,0		; no dec but we can negate and move in one
	popa	1		; so who cares 8)
	mov#	0,0,snr		; check if we have 0 shifts to do
	jmp	done,1
shllp:
	movzl	1,1
	inc	0,0,szr
	jmp	shllp,1
done:	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0

f__shru:
	neg	1,0
	popa	1
shrulp:
	movzr	1,1
	inc	0,0,szr
	jmp	shrulp,1
	jmp	done,1

f__shr:
	neg	1,0
	popa	1
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

;	Same idea but 32bit wide

f__shll:
	neg	1,0
	popa	1		; low
	popa	2		; high
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
	popa	1
	popa	2
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
	popa	1
	popa	2
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
