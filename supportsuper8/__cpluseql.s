	.export __cpluseql

__cpluseql:
	incw rr2
	incw rr2
	incw rr2
	lde r0,@rr2
	add r15,r0
	lde @rr2,r15
	decw rr2
	lde r0,@rr2
	adc r14,r0
	lde @rr2,r14
	decw rr2
	lde r0,@rr2
	adc r13,r0
	lde @rr2,r13
	decw rr2
	lde r0,@rr2
	adc r0,r12
	lde @rr2,r0
	ld r1,r13
	ld r2,r14
	ld r3,r15
	ret
