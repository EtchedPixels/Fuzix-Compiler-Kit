	.export __nref_4
	.export __nstore_4
	.export __nstore_4_0
	.export __nstore_4b


__nref_4:
	pop	r12
	pop	r13
	ldei	r14,@rr12
	ldei	r15,@rr12
	push	r13
	push	r12
	ldei	r0,@rr14
	ldei	r1,@rr14
	ldei	r2,@rr14
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
	ldei	r14,@rr12
	ldei	r15,@rr12
	push	r13
	push	r12
	lde	@rr14,r0
	ldepi	@rr14,r1
	ldepi	@rr14,r2
	ldepi	@rr14,r3
	ret
