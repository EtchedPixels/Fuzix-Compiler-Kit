	.export __cminuseql

__cminuseql:
	add %3,r5
	adc %0,r4

	lda *r5
	sub r13,a
	sta *r5
	mov a,r13
	decd r5

	lda *r5
	sbb r12,a
	sta *r5
	mov a,r12
	decd r5

	lda *r5
	sbb r11,a
	sta *r5
	mov a,b
	decd r5

	lda *r5
	sbb r10,a
	sta *r5
	mov a,r2
	decd r5

	mov b,r3
	movd r13,r5

	rets