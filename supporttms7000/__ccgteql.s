;
;	Comparison between top of stack and ac
;

	.export __ccgteql
	.export __ccgtequl
	.code

__ccgtequl:
	movd r15,r13
	lda *r13
	cmp r2,a
	jnc false
	jmp nbyte
__ccgteql:
	movd r15,r13
	lda *r13
	xor r2,a
	jp samesign
	xor r2,a
	jp false
	jmp true

samesign:
	xor r2,a
	cmp r2,a
	jnc false
nbyte:
	jnz true
	add %1,r13
	adc %0,r12
	lda *r13
	cmp a,r3
	jnc false
	jnz true
	add %1,r13
	adc %0,r12
	lda *r13
	cmp a,r4
	jnc false
	jnz true
	add %1,r13
	adc %0,r12
	lda *r13
	cmp a,r5
	jnc false
true:
	mov %1,r5
out:
	add %4,r15
	adc %0,r14
	clr r4
	or r5,r5	; flags
	rets
false:
	clr r5
	jmp out
