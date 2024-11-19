;
;	Compare r0-r3 with r12-r15
;
	.export __ccneconstl
	.export __ccneconstbl
	.export __ccneconst0l
	.code

__ccneconst0l:
	or	r5,r2
	or	r5,r3
	or	r5,r4
	jnz	true
	; r3 is zero, flags is Z
	rets
__ccneconstbl:
	clr	r10
	clr	r11
	clr	r12
__ccneconstl:
	cmp	r2,r10
	jnz	true
	cmp	r3,r11
	jnz	true
	cmp	r4,r12
	jnz	true
	cmp	r5,r13
	jnz	true
	clr	r4
	clr	r5
	rets
true:
	clr	r4
	mov	%1,r5
	rets
