
	.export __nref12_1
	.export __nref12_2
	.export __nstore12_1
	.export __nstore12_2

__nref12_1:
	pop	r0
	pop	r1
	ldei	r14,@rr0
	ldei	r15,@rr0
	push	r1
	push	r0
	lde	r13,@rr14
	ret

__nref12_2:
	pop	r0
	pop	r1
	ldei	r14,@rr0
	ldei	r15,@rr0
	push	r1
	push	r0
	ldei	r12,@rr14
	lde	r13,@rr14
	ret

__nstore12_1:
	pop	r0
	pop	r1
	ldei	r14,@rr0
	ldei	r15,@rr0
	push	r1
	push	r0
	lde	@rr14,r13
	ret

__nstore12_2:
	pop	r0
	pop	r1
	ldei	r14,@rr0
	ldei	r15,@rr0
	push	r1
	push	r0
	lde	@rr14,r12
	ldepi	@rr14,r13
	ret
