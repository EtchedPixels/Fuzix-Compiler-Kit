;
;	Comparison between top of stack and ac
;

	.export __ccgtl
	.export __ccgtul
	.code

__ccgtul:
	ld r14,254
	ld r15,255
	incw rr14
	incw rr14
	lde r12,@rr14
	cp r0,r12
	jr ult,true
	jr nbyte
__ccgtl:
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
	or r3,r3	; get the flags right
	ret
true:
	ld r3,#1
	jr out
