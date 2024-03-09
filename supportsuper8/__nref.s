
	.export __nref_1
	.export __nref_2
	.export __nstore_1
	.export __nstore_2
	.export __nstore_1_0
	.export __nstore_2_0
	.export __nstore_2b

__nref_1:
	pop	r12
	pop	r13
	ldei	r14,@rr12
	ldei	r15,@rr12
	push	r13
	push	r12
	lde	r3,@rr14
	clr	r2
	ret

__nref_2:
	pop	r12
	pop	r13
	ldei	r14,@rr12
	ldei	r15,@rr12
	push	r13
	push	r12
	ldei	r2,@rr14
	lde	r3,@rr14
	ret
__nstore_1_0:
	clr	r3
__nstore_1:
	pop	r12
	pop	r13
	ldei	r14,@rr12
	ldei	r15,@rr12
	push	r13
	push	r12
	lde	@rr14,r3
	ret

__nstore_2_0:
	clr	r3
__nstore_2b:
	clr	r2
__nstore_2:
	pop	r12
	pop	r13
	ldei	r14,@rr12
	ldei	r15,@rr12
	push	r13
	push	r12
	lde	@rr14,r2
	ldepi	@rr14,r3
	ret
