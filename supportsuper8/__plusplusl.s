;
;	*r2/3 += r12-r15
;
	.export __plusplusl

__plusplusl:
	incw rr2
	incw rr2
	incw rr2
	lde r0,@rr2
	push r0
	add r0,r15
	lde @rr2,r0
	decw rr2
	lde r0,@rr2
	push r0
	adc r0,r14
	lde @rr2,r0
	decw rr2
	lde r0,@rr2
	push r0
	adc r0,r13
	lde @rr2,r0
	decw rr2
	lde r0,@rr2
	push r0
	adc r0,r12
	lde @rr2,r0
	pop r0
	pop r1
	pop r2
	pop r3
	ret
