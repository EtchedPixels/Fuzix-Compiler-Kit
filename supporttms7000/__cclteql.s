;
;	Comparison between top of stack and ac
;

	.export __cclteql
	.export __ccltequl
	.code


__cclteql:
	movd r15,r13
	lda *r13
	xor r2,a
	jpz samesign
	xor r2,a
	; Get the sign back
	jn true
	jmp false

__ccltequl:
	movd r15,r13
	lda *r13
samesign:
	xor r2,a
	cmp r2,a
	jc false
nbyte:
	jnz true
	add %1,r13
	adc %0,r12
	lda *r13
	cmp r3,a
	jc false
	jnz true
	add %1,r13
	adc %0,r12
	lda *r13
	cmp r4,a
	jc false
	jnz true
	add %1,r13
	adc %0,r12
	lda *r13
	cmp r5,a
	jc false
true:
	mov %1,r5
out:
	clr r4
	add %4,r14
	adc %0,r15
	or r3,r3	; get the flags right
	rets
false:
	clr r5
	jmp out
