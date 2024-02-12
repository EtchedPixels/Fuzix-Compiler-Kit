;
;	Comparison between top of stack and ac
;

	.export __cclteql
	.export __ccltequl
	.code

__ccltequl:
	ld r14,254
	ld r15,255
	incw rr14
	incw rr14
	lde r12,@rr14
	cp r0,r12
	jr ult,true
	jr nbyte
__cclteql:
	ld r14,254
	ld r15,255
	incw rr14
	incw rr14	; point to data
	lde r12,@rr14
	cp r0,r12
	jr lt,true
nbyte:
	jr nz,false
	incw rr14
	lde r12,@rr14
	cp r1,r12
	jr ult,true
	jr nz,false
	incw rr14
	lde r12,@rr14
	cp r2,r12
	jr ult,true
	jr nz,false
	incw rr14
	lde r12,@rr14
	cp r3,r12
	jr ult,true
false:
	clr r3
out:
	clr r2
	pop r14		; return address
	pop r15
	add 255,#4
	adc 254,#0
	push r15
	push r14
	xor r3,#1	; invert and get the flags right
	ret
true:
	ld r3,#1
	jr out
