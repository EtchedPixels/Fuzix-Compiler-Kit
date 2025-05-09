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
	.export __garg10r1
	.export	__garg10r2
	.export __garg10rr1
	.export	__garg10rr2
	.export __pargr1
	.export __pargr2
	.export __pargr4
	.export __pargrr1
	.export __pargrr2
	.export __pargrr4
	.export __pargr1_0
	.export __pargr2_0
	.export __pargr4_0
	.export __pargr4_1
	.export __frame
	.export __load2
	.export __load4
	.export __store2
	.export __store4
	.export __garg10r2str
	.export __garg10rr2str

__gargr1:
	clr	r12
__gargrr1:
	add	r15,r13
	adc	r14,r12
	lda	*r13
	mov	a,r5
	rets

__gargr2:
	clr	r12
__gargrr2:
	add	r15,r13
	adc	r14,r12
__load2:
	lda	*r13
	mov	a,r4
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r5
	rets
__load2ac:		; Must leave regs exactly like this
	movd	r13,r3
	lda	*r13
	mov	a,r4
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r5
	rets
__gargr4:
	clr	r12
__gargrr4:
	add	r15,r13
	adc	r14,r12
__load4:
	lda	*r13
	mov	a,r2
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r3
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r4
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r5
	rets

__garg10r1:
	clr	r12
__garg10rr1:
	add	r15,r13
	adc	r14,r12
	lda	*r13
	mov	a,r11
	rets

__garg10r2:
	clr	r12
__garg10rr2:
	add	r15,r13
	adc	r14,r12
	lda	*r13
	mov	a,r10
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r11
	rets

; Rewritten by optimizer as a store helper
__garg10r2str:
	clr	r12
__garg10rr2str:
	add	r15,r13
	adc	r14,r12
	lda	*r13
	mov	a,r10
	add	%1,r13
	adc	%0,r12
	lda	*r13
	mov	a,r11
	rets

__pargr1_0:
	clr	r4
__pargr1:
	clr	r12
__pargrr1:
	add	r15,r13
	adc	r14,r12
	mov	r5,a
	sta	*r13
	rets

__pargr2_0:
	clr	r4
	clr	r5
__pargr2:
	clr	r12
__pargrr2:
	add	r15,r13
	adc	r14,r12
__store2:
	mov	r4,a
	sta	*r13
	add	%1,r13
	adc	%0,r12
	mov	r5,a
	sta	*r13
	rets

__pargr4_1:
	mov	%1,r5
	jmp	pac
__pargr4_0:
	clr	r5
pac:
	clr	r2
	clr	r3
	clr	r4
__pargr4:
	clr	r12
__pargrr4:
	add	r15,r13
	adc	r14,r12
__store4:
	mov	r2,a
	sta	*r13
	add	%1,r13
	adc	%0,r12
	mov	r3,a
	sta	*r13
	add	%1,r13
	adc	%0,r12
	mov	r4,a
	sta	*r13
	add	%1,r13
	adc	%0,r12
	mov	r5,a
	sta	*r13
	rets

	.export __revstore4
	.export __revstore2

__revstore4:
	mov	r5,a
	sta	*r13
	decd	r13
	mov	r4,a
	sta	*r13
	decd	r13
	mov	r3,a
	sta	*r13
	decd	r13
	mov	r2,a
	sta	*r13
	rets

__revstore2:
	mov	r5,a
	sta	*r13
	decd	r13
	mov	r4,a
	sta	*r13
	rets

; Order matters here. Adjust the high byte first so our worst case is
; being a chunk down the stack when we take an interrupt. Stacks are assumed
; to be a number of exact pages long and must allow for this annoying cpu
; limit)
__frame:
	mov	r15,r13
	sub	r13,r11
	sbb	%0,r14	; adjust high first
	mov	r13,r15
	rets

