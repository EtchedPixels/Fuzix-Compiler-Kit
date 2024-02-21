;
;	32bit stack versus accumulator compare. Messier because
;	we don't have the space we need
;
	.export __ccnel
	.code

__ccnel:
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
	clr r3
	; flags is correct for ccne case
	jr out
true:
	ld r3,#1
out:
	clr r2
	pop r14		; return address
	pop r15
	add 255,#4
	adc 254,#0
	push r15
	push r14
	or r3,r3	; get the flags right
	ret
