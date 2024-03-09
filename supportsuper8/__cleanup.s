	.export __cleanup
	.export __cleanupb
	.export __cleanup1
	.export __cleanup2
	.export __cleanup3
	.export __cleanup4
	.export __cleanup5
	.export __cleanup6
	.export __cleanup7
	.export __cleanup8

	.code

__cleanup8:
	ld	r13,#8
	jr	__cleanupb
__cleanup7:
	ld	r13,#7
	jr	__cleanupb
__cleanup6:
	ld	r13,#6
	jr	__cleanupb
__cleanup5:
	ld	r13,#5
	jr	__cleanupb
__cleanup3:
	ld	r13,#3
	jr	__cleanupb
__cleanup4:			; 4 is common, 3 is not
	ld	r13,#4
__cleanupb:
	clr	r12
__cleanup:
	pop	r14
	pop	r15
	add	217,r13
	adc	216,r12
	push	r15
	push	r14
	ret
__cleanup1:
	pop	r14
	pop	r15
	incw	216
	push	r15
	push	r14
	ret
__cleanup2:
	pop	r14
	pop	r15
	incw	216
	incw	216
	push	r15
	push	r14
	ret
