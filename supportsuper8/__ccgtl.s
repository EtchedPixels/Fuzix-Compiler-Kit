;
;	Comparison between top of stack and ac
;

	.export __ccgtl
	.export __ccgtul
	.code

__ccgtul:
	ldw rr14,216
	incw rr14
	incw rr14
	ldei r12,@rr14
	cp r0,r12
	jr ult,true
	jr nbyte
__ccgtl:
	ldw rr14,216
	incw rr14
	incw rr14	; point to data
	ldei r12,@rr14
	cp r0,r12
	jr lt,true
nbyte:
	jr nz,false
	ldei r12,@rr14
	cp r1,r12
	jr ult,true
	jr nz,false
	ldei r12,@rr14
	cp r2,r12
	jr ult,true
	jr nz,false
	lde r12,@rr14
	cp r3,r12
	jr ult,true
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
true:
	ld r3,#1
	jr out
