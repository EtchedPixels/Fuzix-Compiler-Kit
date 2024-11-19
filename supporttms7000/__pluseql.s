;
;	*TOS + AC
;
	.export __pluseql
	.code


__pluseql:
	call @__pop12

	add %3,r13
	adc %0,r12	;	point to low byte

	lda *r13
	add a,r5
	mov r5,a
	sta *r13
	decd r13

	lda *r13
	adc a,r4
	mov r5,a
	sta *r13
	decd r13

	lda *r13
	adc a,r3
	mov r5,a
	sta *r13
	decd r13

	lda *r13
	adc a,r2
	mov r5,a
	sta *r13

	rets

