	.export __cpluseq
	.export __cpluseqc
	.code

__cpluseq:
	lde r0,@rr2
	incw rr2
	lde r1,@rr2
	add r1,r13
	adc r0,r12
	lde @rr2,r1
	decw rr2
	lde @rr2,r0
	ld r2,r0
	ld r3,r1
	ret

__cpluseqc:
	lde r0,@rr2
	add r0,r12
	lde @rr2,r0
	ld r3,r0
	ret

