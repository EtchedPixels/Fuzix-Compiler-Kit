;
;	32bit stack versus accumulator compare. Messier because
;	we don't have the space we need
;
	.export __cceql
	.code

__cceql:
	ld r15,255
	ld r14,254
	add r15,#2
	adc r14,#0		; index now points to the bytes to compare

	lde r13,@rr14
	cp r13, r0
	jr nz, true
	lde r13,@rr14
	cp r13, r1
	jr nz, true
	lde r13,@rr14
	cp r13, r2
	jr nz, true
	lde r13,@rr14
	cp r13, r3
	jr nz, true
	clr r2
	clr r3
	or r3,r3
	ret
true:
	clr r2
	ld r3,#1
	or r3,r3
	ret
