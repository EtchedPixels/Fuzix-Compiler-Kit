	.export __cminuseq
	.export __cminuseqc
	.code

__cminuseq:
	ldei r0,@rr2
	ldei r1,@rr2
	sub r1,r13
	sbc r0,r12
	ldepd @rr2,r1
	lde @rr2,r0
	ld r2,r0
	ld r3,r1
	ret

__cminuseqc:
	lde r0,@rr2
	sub r0,r12
	lde @rr2,r0
	ld r3,r0
	ret

