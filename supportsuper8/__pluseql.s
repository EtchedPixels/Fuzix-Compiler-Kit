;
;	*TOS + AC
;
	.export __pluseql
	.code


__pluseql:
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
	add r3,r12
	lde @rr14,r3
	decw rr14

	lde r12,@rr14
	adc r2,r12
	lde @rr14,r2
	decw rr14

	lde r12,@rr14
	adc r1,r12
	lde @rr14,r1
	decw rr14

	lde r12,@rr14
	adc r0,r12
	lde @rr14,r0

	ret
