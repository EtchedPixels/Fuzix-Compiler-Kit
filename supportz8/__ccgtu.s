;
;	Compare top of stack ac
;
	.export __ccgtu
	.export __ccgtconstu
	.export __ccgtconst0u
	.export __ccgtconstbu
	.code

__ccgtconst0u:
	clr r13
__ccgtconstbu:
	clr r12
	jr __ccgtconstu
__ccgtu:
	pop r14		; Return address
	pop r15
	pop r12		; value for comparison
	pop r13
	push r15
	push r14
__ccgtconstu:
	cp r12,r2
	clr r2
	jr nz, c1
	cp r13,r3
	jr z, iseq
c1:
	; if C set then less than
	ccf
	clr r3
	adc r3,#0
	ret
iseq:
	clr r3
	or r3,r3
	ret
