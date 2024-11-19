;
;	AC = TOS - AC and remove from stack
;
;
	.export __minusl
	.code

__minusl:
	ld	r15,255
	ld	r14,254
	add	r15,#5
	adc	r14,#0

	; Now have to access them as memory
	lde	r13,@rr14
	sub	r13,r5
	ld	r5,r13
	decw	rr14
	lde	r13,@rr14
	sbc	r13,r4
	ld	r4,r13
	decw	rr14
	lde	r13,@rr14
	sbc	r13,r3
	ld	r3,r13
	decw	rr14
	lde	r13,@rr14
	sbc	r13,r2
	ld	r2,r13

	; Now clean up
	pop	r15
	pop	r14
	pop	r13
	pop	r13
	pop	r13
	pop	r13
	push	r14
	push	r15
	ret
	