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
	clr r2
	lde r1, @rr12
	mult rr2,r1
	lde @rr12,r3
	clr r2
	ret
