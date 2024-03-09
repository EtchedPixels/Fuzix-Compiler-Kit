;
;	AC = TOS + AC and remove from stack
;
	.export __plusl
	.code

__plusl:
	pop r14		; return address
	pop r15
	pop r13		; low byte
	add r3,r13
	pop r13
	adc r2,r13
	pop r13
	adc r1,r13
	pop r13
	adc r0,r13
	push r15
	push r14
	ret
