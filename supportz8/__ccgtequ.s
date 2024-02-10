;
;	Compare top of stack ac
;
	.export __ccgtequ
	.export __ccgteqconstu
	.code

__ccgtequ:
	pop r14		; Return address
	pop r15
	pop r12		; value for comparison
	pop r13
	push r15
	push r14
__ccgteqconstu:
	cp r12,r2
	jr nz, c1
	cp r13,r3
c1:
	; if C set then less than
	ccf
	clr r2
	clr r3
	adc r3,#0
	ret
