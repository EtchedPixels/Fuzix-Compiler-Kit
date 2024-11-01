
	.export __pnref_2
	.code

__pnref_2:
	pop	r12
	pop	r13
	lde	r14,@rr12
	incw	rr12
	lde	r15,@rr12
	incw 	rr12
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	push	r3
	push	r2
	push	r13
	push	r12
	ret
