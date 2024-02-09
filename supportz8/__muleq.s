;
;	Same helper but with load/store from pointer
;
	.export __muleq
	.export __mulequ   	

	.code

__muleq:
__mulequ:
	; stack holds ptr instead in this case
	pop r14
	pop r15
	pop r12		; address
	pop r13
	push r15
	push r14

	; Get values
	lde r0, @rr12
	incw rr12
	lde r1, @rr12

	; save ptr
	push r13
	push r12

	; r0,r1 x r2,r3
	call __domul

	; result in r2,r3, r12/r13/14 trashed
	pop r12
	pop r13
	lde @rr12,r3
	decw rr12
	lde @rr12,r2
	ret

