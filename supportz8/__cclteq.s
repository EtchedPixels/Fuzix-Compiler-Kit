;
;	Signed 16bit compare with top of stack
;
	.export __cclteq
	.export __cclteqconst
	.export __cclteqconst0
	.export __cclteqconstb
	.code

__cclteqconst0:
	clr r13
__cclteqconstb:
	clr r12
	jr __cclteqconst
__cclteq:
	pop r14
	pop r15
	pop r12
	pop r13
	push r15
	push r14
__cclteqconst:
	cp r2,r12
	clr r2
	jr nz, diff1
	cp r3,r13
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
