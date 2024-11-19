;
;	Compare r2-r5 with r10-r13
;
	.export __cceqconstl
	.export __cceqconstbl
	.export __cceqconst0l
	.code

__cceqconst0l:
	or	r5,r2
	or	r5,r3
	or	r5,r4
	jnz	false
	clr	r4
	; r3 is 0
	inc	r5	; force to NZ
	rets
__cceqconstbl:
	clr	r10
	clr	r11
	clr	r12
__cceqconstl:
	cmp	r2,r10
	jnz	false
	cmp	r3,r11
	jnz	false
	cmp	r4,r12
	jnz	false
	cmp	r5,r13
	jnz	false
	clr	r4
	mov	%1,r5
	rets
false:
	clr	r4
	clr	r5
	rets
