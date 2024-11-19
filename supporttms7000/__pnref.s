
	.export __pnref_2
	.code

__pnref_2:
	; Pointer is in r12/r13
	lda	@r13
	mov	a,b
	add	%1,r13
	adc	%0,r12
	lda	@r13
	decd	r15
	sta	@r15
	mov	a,b
	decd	r15
	sta	@r15
	rets

