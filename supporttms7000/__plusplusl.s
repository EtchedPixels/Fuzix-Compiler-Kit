;
;	*r4/5 += r10-r13
;
	.export __plusplusl

__plusplusl:
	add %3,r5
	adc %0,r4	; point at low end
	lda *r5
	mov a,b
	add r13,a
	sta *r5
	decd r5
	lda *r5
	mov a,r13
	add r12,a
	sta *r5
	decd r5
	lda *r5
	mov a,r3
	add r11,a
	sta *r5
	decd r5
	lda *r5
	mov a,r2
	add r10,a
	sta *r5
	mov b,r5
	mov r13,r4
	rets


