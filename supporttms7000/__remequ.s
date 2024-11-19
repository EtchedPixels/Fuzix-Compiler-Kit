	.export __remequ
	.code

__remequ:
	call @__pop12
	lda *r13
	mov a,r4
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r5
	push r13
	push r12
	call @__div16x16	; do the division
	pop r12
	pop r13
	movd r13,r5
	mov r5,a
	sta *r13
	decd r13
	mov r4,a
	sta *r13
	rets


