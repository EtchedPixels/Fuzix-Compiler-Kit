;
;	AC = TOS - AC and remove from stack
;
	.export __minusl
	.code

__minusl:
	pop r14		; return address
	pop r15
	pop r13		; low byte
	sub r13,r3
	ld r3,r13
	pop r13
	sbc r13,r2
	ld r2,r13
	pop r13
	sbc r13,r1
	ld r1,r13
	pop r13
	sbc r13,r0
	ld r0,r13
	push r15
	push r14
	ret
