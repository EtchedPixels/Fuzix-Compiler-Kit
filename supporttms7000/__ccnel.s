;
;	32bit stack versus accumulator compare. Messier because
;	we don't have the space we need
;
	.export __ccnel
	.code

__ccnel:
	movd r15,r13
	add %4,r13
	adc %0,r12
	mov r13,r11
	decd r13
	lda *r13
	cmp a, r5
	jnz true
	decd r13
	lda *r13
	cmp a, r4
	jnz true
	decd r13
	lda *r13
	cmp a, r3
	jnz true
	decd r13
	lda *r13
	cmp a, r2
	jnz true
	clr r5
	jmp out
true:
	mov %1,r5
out:
	clr r4
	movd r11,r15
	rets

