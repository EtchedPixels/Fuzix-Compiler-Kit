
	.export __nref10_1
	.export __nref10_2
	.export __nstore10_1
	.export __nstore10_2

__nref10_1:
	lda	*r13
	mov	a,r11
	clr	r10
	rets

__nref10_2:
	lda	*r13
	mov	a,r11
	decd	r13
	lda	*r13
	mov	a,r10
	rets

__nstore10_1:
	mov	r11,a
	sta	*r13
	rets

__nstore10_2:
	mov	r11,a
	sta	*r13
	decd	r13
	mov	r10,a
	sta	*r13
	rets
