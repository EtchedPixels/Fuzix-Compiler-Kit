;
;	Comparison between top of stack and ac
;

	.export __ccltl
	.export __ccltul
	.code

__ccltul:
	movd r15,r13
	lda *r13
	cmp r2,a
	jnc true
	jmp nbyte
__ccltl:
	movd r15,r13
	lda *r13
	xor r2,a
	jp samesign
	xor r2,a
	jn true
	jmp false

samesign:
	xor r2,a
	cmp r2,a
	jnc true
nbyte:
	jnz false
	add %1,r13
	adc %0,r12
	lda *r13
	cmp a,r3
	jnc true
	jnz false
	add %1,r13
	adc %0,r12
	lda *r13
	cmp a,r4
	jnc true
	jnz false
	add %1,r13
	adc %0,r12
	lda *r13
	cmp a,r5
	jnc true
false:
	clr r5
out:
	add %4,r15
	adc %0,r14
	clr r4
	or r5,r5	; flags
	rets
true:
	mov %1,r5
	jmp out
