;
;	Compare top of stack ac
;
	.export __ccltequ
	.export __cclteqconstu
	.export __cclteqconst0u
	.export __cclteqconstbu
	.code

__cclteqconst0u:
	clr r13
__cclteqconstbu:
	clr r12
	jr __cclteqconstu
__ccltequ:
	pop r14		; Return address
	pop r15
	pop r12		; value for comparison
	pop r13
	push r15
	push r14
__cclteqconstu:
	cp r12,r2
	clr r2
	jr nz, c1
	cp r13,r3
	jr z, iseq
c1:
	; if C set then less than
	clr r3
	adc r3,#0
	ret
iseq:
	ld r3,#1
	or r3,r3
	ret
