;
;	Argument stack shorteners
;

	.export __pushl0
	.export __pushl0a
	.export __pushln
	.export __pushlnl
	.export __pushl
	.export __pushw
	.export	__push0
	.export	__push1

__pushlnl:
	movd	r15,r13
	add	r11,r13
	adc	%3,r12

	lda	*r13
	mov	a,r5
	decd	r13
	lda	*r13
	mov	a,r4
	lda	*r13
	mov	a,r3
	decd	r13
	lda	*r13
	mov	a,r2
	jmp	__pushl
__pushl0:
	clr	r4
	clr	r5
__pushl0a:
	clr	r2
	clr	r3
__pushl:
	decd	r13
	mov	r5,a
	sta	*r13
	dec	r13
	mov	r4,a
	sta	*r13
	decd	r13
	mov	r3,a
	sta	*r13
	dec	r13
	mov	r2,a
	sta	*r13
	rets
__pushln:
	movd	r15,r13
	add	r11,r13
	adc	%1,r12

	lda	*r13
	mov	a,r5
	decd	r13
	lda	*r13
	mov	a,r4
	jmp	__pushw
__push1:
	mov	%1,r5
	jmp	__push0r
__push0:
	clr	r5
__push0r:
	clr	r4
__pushw:
	decd	r13
	mov	r5,a
	sta	*r13
	decd	r13
	mov	r4,a
	sta	*r13
	rets
