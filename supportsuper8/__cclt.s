;
;	Signed 16bit compare with top of stack
;
	.export __cclt
	.export __ccltconst
	.export __ccltconst0
	.export __ccltconstb
	.code

__ccltconst0:
	clr r13
__ccltconstb:
	clr r12
	jr __ccltconst
__cclt:
	pop r14
	pop r15
	pop r12
	pop r13
	push r15
	push r14
__ccltconst:
	cp r12,r2
	clr r2
	jr nz, diff1
	cp r13,r3
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
