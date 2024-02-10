;
;	Compare top of stack ac
;
	.export __ccne
	.export __ccneconst
	.code

__ccne:
	pop r14		; Return address
	pop r15
	pop r12		; value for comparison
	pop r13
	push r15
	push r14
__ccneconst :
	cp r12,r2
	clr r2
	jr nz, c1
	cp r13,r3
	jr nz, c1
	clr r3
	or r3,r3
	ret
c1:	ld r3,#1
	or r3,r3
	ret
