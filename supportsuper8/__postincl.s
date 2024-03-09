;
;	*TOS - AC
;
	.export __postincl
	.code


__postincl:
	pop r12		;	return
	pop r13
	pop r14		;	pointer (TODO: pass ptr in call ?)
	pop r15
	push r13
	push r12

	incw rr14
	incw rr14
	incw rr14	;	point to low byte

	lde r12,@rr14
	ld r13,r12
	add r12,r3
	ld r3,r13
	lde @rr14,r12
	decw rr14

	lde r12,@rr14
	ld r13,r12
	adc r12,r2
	ld r2,r13
	lde @rr14,r12
	decw rr14

	lde r12,@rr14
	ld r13,r12
	adc r12,r1
	ld r1,r13
	lde @rr14,r12
	decw rr14

	lde r12,@rr14
	ld r13,r12
	adc r12,r0
	ld r0,r13
	lde @rr14,r12

	ret
