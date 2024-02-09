	.export __muleqc
	.export __mulequc   	

	.code

__muleqc:
__mulequc:
	; stack holds ptr instead in this case
	pop r14
	pop r15
	pop r12		; address
	pop r13
	push r15
	push r14

	; Get values
	clr r0
	lde r1, @rr12

	; save ptr
	push r13
	push r12

	; r0,r1 x r2,r3
	call __domul

	; result in r2,r3, r12/r13/14 trashed
	pop r12
	pop r13
	lde @rr12,r3
	clr r2
	ret
