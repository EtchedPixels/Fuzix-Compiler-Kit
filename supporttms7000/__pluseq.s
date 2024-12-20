;
;	*TOS + AC
;
	.export __pluseq
	.code


__pluseq:
	call @__pop12	;	TODO pass pointer into call via r12/r13

	add %1,r13
	adc %0,r12

	lda *r13
	add a,r5
	mov r5,a
	sta *r13
	decd r13

	lda *r13
	adc a,r4
	mov r4,a
	sta *r13

	rets
