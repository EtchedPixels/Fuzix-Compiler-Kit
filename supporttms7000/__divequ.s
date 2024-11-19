	.export __divequ
	.code
__divequ:
	call @__pop12
	lda *r13
	mov a,r2
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r3
	push r12
	push r13
	call @__div16x16	; do the division
	pop r13
	pop r12
	movd r3,r5	; we want the quotient
	mov r3,a
	sta *r13
	decd r13
	movd r2,a
	sta *r13
	rets

