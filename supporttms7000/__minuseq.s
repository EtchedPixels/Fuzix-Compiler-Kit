;
;	*TOS - AC
;
	.export __minuseq
	.code


__minuseq:
	call @__pop12
	add %1,r13
	adc %0,r12

	lda @r13
	sub r5,a
	sta @r13
	mov a,r5
	decd r13

	lda @r13
	sub r5,a
	sta @r13
	mov a,r5

	rets
