	.export __boolc
	.export __bool
	.export __booll

	.code

__booll:
	or	r2,r4
	or	r3,r4
__bool:
	or	r4,r5
__boolc:
	clr	r4
	or	r5,r5
	jnz	set1
	rets
set1:
	mov	%1,r5
	rets
