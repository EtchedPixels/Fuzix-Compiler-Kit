;
;	Helper for size/low opt settings to do loads
;
;	These functions all only affect the accumulator and r14/15
;	They *must* leave r14/15 where the compiler expects as it has
;	internal knowledge of this
;
	.export __gargr1
	.export __gargr2
	.export __gargr4
	.export __gargrr1
	.export __gargrr2
	.export __gargrr4
	.export __garg12r1
	.export	__garg12r2
	.export __garg12rr1
	.export	__garg12rr2
	.export __pargr1
	.export __pargr2
	.export __pargr4
	.export __pargrr1
	.export __pargrr2
	.export __pargrr4
	.export __frame
	.export __load2
	.export __load4
	.export __store2
	.export __store4

__gargr1:
	clr	r14
__gargrr1:
	add	r15,255
	adc	r14,254
	lde	r3,@rr14
	ret

__gargr2:
	clr	r14
__gargrr2:
	add	r15,255
	adc	r14,254
__load2:
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	ret

__gargr4:
	clr	r14
__gargrr4:
	add	r15,255
	adc	r14,254
__load4:
	lde	r0,@rr14
	incw	rr14
	lde	r1,@rr14
	incw	rr14
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	ret

__garg12r1:
	clr	r14
__garg12rr1:
	add	r15,255
	adc	r14,254
	lde	r13,@rr14
	ret

__garg12r2:
	clr	r14
__garg12rr2:
	add	r15,255
	adc	r14,254
	lde	r12,@rr14
	incw	rr14
	lde	r13,@rr14
	ret

__pargr1:
	clr	r14
__pargrr1:
	add	r15,255
	adc	r14,254
	lde	@rr14,r3
	ret

__pargr2:
	clr	r14
__pargrr2:
	add	r15,255
	adc	r14,254
__store2:
	lde	@rr14,r2
	incw	rr14
	lde	@rr14,r3
	ret
__pargr4:
	clr	r14

__pargrr4:
	add	r15,255
	adc	r14,254
__store4:
        lde	@rr14,r0
	incw	rr14
	lde	@rr14,r1
	incw	rr14
	lde	@rr14,r2
	incw	rr14
	lde	@rr14,r3
	ret

	.export __revstore4
	.export __revstore2

__revstore4:
	lde	@rr14,r3
	decw	rr14
	lde	@rr14,r2
	decw	rr14
	lde	@rr14,r1
	decw	rr14
	lde	@rr14,r0
	ret

__revstore2:
	lde	@rr14,r3
	decw	rr14
	lde	@rr14,r2
	ret

; Order matters here. Adjust the high byte first so our worst case is
; being a chunk down the stack when we take an interrupt. Stacks are assumed
; to be a number of exact pages long and must allow for this annoying cpu
; limit)
__frame:
	pop	r3
	pop	r2
	ld	r15, 255
	sub	r15, r13
	sbc	254,#0
	ld	255,r15
	push	r2
	push	r3
	ret
