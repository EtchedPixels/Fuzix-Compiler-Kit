;
;	32bit stack versus accumulator compare. Messier because
;	we don't have the space we need
;
	.export __cceql
	.code

__cceql:
	ldei r13,@rr14
	cp r13, r0
	jr nz, false
	ldei r13,@rr14
	cp r13, r1
	jr nz, false
	ldei r13,@rr14
	cp r13, r2
	jr nz, false
	lde r13,@rr14
	cp r13, r3
	jr nz, false
	ld r3,#1
	jr out
false:
	clr r3
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
