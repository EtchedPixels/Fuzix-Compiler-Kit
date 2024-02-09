	.export __divequ
	.code
__divequ:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	ld r2,r12	; we want the quotient
	ld r3,r13
	lde @rr14,r3
	decw rr14
	lde @rr14,r2
	ret
