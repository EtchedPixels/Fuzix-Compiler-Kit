;
;	32bit stack versus accumulator compare. Messier because
;	we don't have the space we need
;
	.export __ccnel
	.code

__ccnel:
	ldw rr14,216
	incw rr14
	incw rr14

	ldei r13,@rr14
	cp r13, r0
	jr nz, true
	ldei r13,@rr14
	cp r13, r1
	jr nz, true
	ldei r13,@rr14
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
	add 217,#4
	adc 216,#0
	push r15
	push r14
	or r3,r3	; get the flags right
	ret
