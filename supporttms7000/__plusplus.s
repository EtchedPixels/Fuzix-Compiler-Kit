	.export __plusplus
	.export __plusplusc
	.code
;
;	(r4) + r12/13
;	return original data
;
__plusplus:
	lda *r4
	mov a,r3
	add %1,r5
	adc %0,r4
	lda *r4
	mov a,r2
	; R2/R3 is the original
	add r2,r12
	adc r3,r13
	mov r13,a
	sta *r13
	decd r4
	mov r12,a
	sta *r13
	movd r3,r5
	rets

__plusplusc:
	lda *r4
	mov a,b
	add r13,a
	sta *r4
	mov b,r5
	clr r4
	rets

	
