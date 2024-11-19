	.export __cpluseql

__cpluseql:
	add %3,r5
	adc %0,r4

	lda *r5
	add r13,a
	sta *r5
	decd r5
	mov a,r13

	lda *r5
	adc r12,a
	sta *r5
	decd r5
	mov a,r12

	lda *r5
	adc r11,a
	sta *r5
	decd r5
	mov a,b

	lda *r5
	adc r10,a
	sta *r5
	decd r5

	mov a,r5
	mov b,r4
	movd r13,r3

	rets
