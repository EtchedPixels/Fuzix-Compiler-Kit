;
;	++ on a local at offset r15
;
	.export __lplusplus
	.export __lplusplusc
	.export __lplusplusb
	.export __lplusplusbc
	.export __lplusplus1
	.export __lplusplus1c
	.code

__lplusplus1:
	ld r3,#1
__lplusplusb:
	clr r2
__lplusplus:
	clr r14
	add r15,255
	adc r14,254
	; Pointer is now in r14/r15 and to last byte, sum to add in r2/r3
	lde r0,@rr14
	push r0
	add r0,r3
	lde @rr14,r0
	decw rr14
	lde r0,@rr14
	adc r2,r0
	lde @rr14,r2
	ld r2,r0
	pop r3
	ret

__lplusplus1c:
	ld r3,#1
__lplusplusbc:
__lplusplusc:
	clr r14
	add r15,255
	adc r14,254
	; Pointer is now in r14/r15, sum to add in r2/r3
	lde r0,@rr14
	add r3,r0
	lde @rr14,r3
	ld r3,r0
	ret

