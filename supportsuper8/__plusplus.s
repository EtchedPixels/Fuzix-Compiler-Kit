	.export __plusplus
	.export __plusplusc
	.code

__plusplus:
	ldei r0,@rr2
	lde r1,@rr2
	push r1
	push r0
	add r1,r13
	adc r0,r12
	lde @rr2,r1
	ldepd @rr2,r0
	pop r2
	pop r3
	ret

__plusplusc:
	lde r0,@rr2
	push r0
	add r0,r12
	lde @rr2,r0
	pop r3
	ret

