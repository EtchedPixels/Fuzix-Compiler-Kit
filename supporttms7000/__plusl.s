;
;	AC = TOS + AC and remove from stack
;
	.export __plusl
	.code

__plusl:
	movd r15,r13	; copy stack pointer - stuff is those 4 bytes
	add %3,r13
	adc %0,r12
	movd r13,r11	; save new sp
	lda *r13
	add a,r5
	decd r13
	lda *r13
	adc a,r4
	decd r13
	lda *r13
	adc a,r3
	decd r13
	lda *r13
	adc a,r2
	movd r11,r15	; clean stack
	rets
