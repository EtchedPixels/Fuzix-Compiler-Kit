;
;	Compare top of stack ac
;
	.export __ccltu
	.export __ccltconstu
	.export __ccltconst0u
	.code

__ccltconst0u:
	clr r2
	clr r3
	jr __ccltconstu
__ccltu:
	pop r14		; Return address
	pop r15
	pop r12		; value for comparison
	pop r13
	push r15
	push r14
__ccltconstu:
	cp r12,r2
	jr nz, c1
	cp r13,r3
c1:
	; if C set then less than
	clr r2
	clr r3
	adc r3,#0
	ret
