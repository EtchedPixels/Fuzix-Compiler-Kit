;
;	Argument stack shorteners
;

	.export __pushl0
	.export __pushl0a
	.export __pushln
	.export __pushlnl
	.export __pushl
	.export __pushw
	.export	__push0
	.export	__push1
__pushlnl:
	clr	r14
	add	r15,255
	adc	r14,254
	lde	r0,@rr14
	incw	rr14
	lde	r1,@rr14
	incw	rr14
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	jr	__pushl
__pushl0:
	clr	r2
	clr	r3
__pushl0a:
	clr	r0
	clr	r1
__pushl:
	pop	r12
	pop	r13
	push	r3
	push	r2
	push	r1
	push	r0
	push	r13
	push	r12
	ret
__pushln:
	clr	r14
	add	r15,255
	adc	r14,254
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	jr	__pushw
__push1:
	ld	r3,#1
	jr	__push0r
__push0:
	clr	r3
__push0r:
	clr	r2
__pushw:
	pop	r12
	pop	r13
	push	r3
	push	r2
	push	r13
	push	r12
	ret
