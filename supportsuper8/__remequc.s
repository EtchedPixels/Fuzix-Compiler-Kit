
	.export __remequc
	.code
__remequc:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	clr r0
	lde r1,@rr14
	clr r2
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	clr r2
	ld r3,r13
	lde @rr14,r3
	ret
