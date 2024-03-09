	.export __divequ
	.code
__divequ:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	ldei r0,@rr14
	ldei r1,@rr14
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	ld r2,r0	; we want the quotient
	ld r3,r1
	ldepd @rr14,r3
	ldepd @rr14,r2
	ret
