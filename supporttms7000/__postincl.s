;
;	*TOS + AC
;
	.export __postincl
	.code


__postincl:
	call @__pop12

	add %3,r13
	adc %0,r12

	lda *r13
	mov a,b
	add r5,a
	sta *r13
	mov b,r5
	decd r13

	lda *r13
	mov a,b
	add r4,a
	sta *r13
	mov b,r4
	decd r13

	lda *r13
	mov a,b
	add r3,a
	sta *r13
	mov b,r3
	decd r13

	lda *r13
	mov a,b
	add r2,a
	sta *r13
	mov b,r2

	rets
