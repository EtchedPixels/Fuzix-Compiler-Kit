;
;	Signed 16bit compare with top of stack
;
	.export __ccgteq
	.export __ccgteqconst
	.code

__ccgteq:
	pop r14
	pop r15
	pop r12
	pop r13
	push r15
	push r14
__ccgteqconst:
	cp r12,r2
	clr r2
	jr nz, diff1
	cp r13,r3
	clr r3
	ccf
	adc r3,#0
	ret
diff1:
	clr r3
	; Signed compare for first byte of check
	jr lt, rz
	ld r3,#1
rz:
	or r3,r3
	ret
