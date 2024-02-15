	.export __nref_4
	.export __nstore_4
	.export __nstore_4_0
	.export __nstore_4b


__nref_4:
	pop	r12
	pop	r13
	lde	r14,@rr12
	incw	rr12
	lde	r15,@rr12
	incw 	rr12
	push	r13
	push	r12
	lde	r0,@rr14
	incw	rr14
	lde	r1,@rr14
	incw	rr14
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	ret

__nstore_4_0:
	clr	r3
__nstore_4b:
	clr	r2
	clr	r1
	clr	r0
__nstore_4:
	pop	r12
	pop	r13
	lde	r14,@rr12
	incw	rr12
	lde	r15,@rr12
	incw 	rr12
	push	r13
	push	r12
	lde	@rr14,r0
	incw	rr14
	lde	@rr14,r1
	incw	rr14
	lde	@rr14,r2
	incw	rr14
	lde	@rr14,r3
	ret
