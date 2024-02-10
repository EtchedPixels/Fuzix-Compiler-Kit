	.export __boolc
	.export __bool
	.export __booll

	.code

__booll:
	or	r3,r0
	or	r3,r1
__bool:
	or	r3,r2
__boolc:
	clr	r2
	or	r3,r3
	jr	nz, set1
	ret
set1:
	ld	r3,#1
	ret
