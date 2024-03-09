;
;	++ on a local at offset r15
;
	.export __lplusplusl
	.export __lplusplusbl
	.export __lplusplus1l
	.code

;
;	r15 points to the last byte
;
__lplusplus1l:
	ld r3,#1
__lplusplusbl:
	clr r0
	clr r1
	clr r2
__lplusplusl:
	clr r14
	add r15,217
	adc r14,216
	; Pointer is now in r14/r15, sum to add in r2/r3
	lde r12,@rr14
	push r12
	add r12,r3
	lde @rr14,r12
	decw rr14
	lde r12,@rr14
	push r12
	adc r12,r2
	lde @rr14,r12
	decw rr14
	lde r12,@rr14
	push r12
	add r12,r1
	lde @rr14,r12
	decw rr14
	lde r12,@rr14
	adc r0,r12
	lde @rr14,r0
	ld r0,r12
	pop r1
	pop r2
	pop r3
	ret

