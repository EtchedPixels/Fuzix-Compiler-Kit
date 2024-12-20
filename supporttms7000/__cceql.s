;
;	32bit stack versus accumulator compare. Messier because
;	we don't have the space we need
;
	.export __cceql
	.code

__cceql:
	movd r15,r13
	add %4,r13
	adc %0,r12
	mov r13,r11
	decd r13
	lda *r13
	cmp a, r5
	jnz false
	decd r13
	lda *r13
	cmp a, r4
	jnz false
	decd r13
	lda *r13
	cmp a, r3
	jnz false
	decd r13
	lda *r13
	cmp a, r2
	jnz false
	mov %1,r5
	jmp out
false:
	clr r5
out:
	clr r4
	movd r11,r15
	rets

