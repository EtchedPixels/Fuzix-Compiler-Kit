;
;	Comparison between top of stack and ac
;

	.export __ccgtl
	.export __ccgtul
	.code

__ccgtul:
	movd r15,r13
	lda *r13
	cmp r2,a
	jc true
	jmp nbyte
__ccgtl:
	movd r15,r13
	lda *r13
	cmp r2,a
	;jr lt,true	FIXME
nbyte:
	jnz false
	add %1,r13
	adc %0,r12
	lda *r13
	cmp r3,a
	jc true
	jnz false
	add %1,r13
	adc %0,r12
	lda *r13
	cmp r4,a
	jc true
	jnz false
	add %1,r13
	adc %0,r12
	lda *r13
	cmp r5,a
	jc true
false:
	clr r5
out:
	clr r4
	add %4,r14
	adc %0,r15
	or r3,r3	; get the flags right
	rets
true:
	mov %1,r5
	jmp out
