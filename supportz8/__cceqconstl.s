;
;	Compare r0-r3 with r12-r15
;
	.export __cceqconstl
	.code

__cceqconstl:
	cp	r0,r12
	jr	nz,false
	cp	r1,r13
	jr	nz,false
	cp	r2,r14
	jr	nz,false
	cp	r3,r15
	jr	nz,false
	clr	r2
	ld	r3,#1
	or	r3,r3
	ret
false:
	clr	r2
	xor	r3,r3
	ret
