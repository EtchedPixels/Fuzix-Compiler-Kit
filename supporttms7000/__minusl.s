;
;	AC = TOS - AC and remove from stack
;
	.export __minusl
	.code

__minusl:
	; TOS holds the value but it's high byte first
	movd r15,r13
	add %4,r13
	adc %0,r12
	; point to the new tos and save it
	movd r13,r11
	decd r13
	lda *r13
	sub a,r5
	mov a,r5
	decd r13
	lda *r13
	sbb a,r4
	mov a,r4
	decd r13
	lda *r13
	sub a,r3
	mov a,r3
	decd r13
	lda *r12
	sbb a,r2
	mov a,r2
	movd r11,r14	; and adjust the stack
	rets

