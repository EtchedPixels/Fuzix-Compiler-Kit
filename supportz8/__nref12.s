
	.export __nref12_1
	.export __nref12_2
	.export __nstore12_1
	.export __nstore12_2

__nref12_1:
	pop	r0
	pop	r1
	lde	r14,@rr0
	incw	rr0
	lde	r15,@rr0
	incw 	rr0
	push	r1
	push	r0
	lde	r13,@rr14
	ret

__nref12_2:
	pop	r0
	pop	r1
	lde	r14,@rr0
	incw	rr0
	lde	r15,@rr0
	incw 	rr0
	push	r1
	push	r0
	lde	r12,@rr14
	incw	rr14
	lde	r13,@rr14
	ret

__nstore12_1:
	pop	r0
	pop	r1
	lde	r14,@rr0
	incw	rr0
	lde	r15,@rr0
	incw 	rr0
	push	r1
	push	r0
	lde	@rr14,r13
	ret

__nstore12_2:
	pop	r0
	pop	r1
	lde	r14,@rr0
	incw	rr0
	lde	r15,@rr0
	incw 	rr0
	push	r1
	push	r0
	lde	@rr14,r12
	incw	rr14
	lde	@rr14,r13
	ret
