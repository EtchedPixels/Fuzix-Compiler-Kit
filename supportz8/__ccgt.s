;
;	Signed 16bit compare with top of stack
;
	.export __ccgt
	.export __ccgtconst
	.export __ccgtconst0
	.code

__ccgtconst0:
	clr r12
	clr r13
	jr __ccgtconst
__ccgt:
	pop r14
	pop r15
	pop r12
	pop r13
	push r15
	push r14
__ccgtconst:
	cp r2,r12
	clr r2
	jr nz, diff1
	cp r3,r13
	clr r3
	adc r3,#0
	ret
diff1:
	clr r3
	; Signed compare for first byte of check
	jr ge, rz
	ld r3,#1
rz:
	or r3,r3
	ret
