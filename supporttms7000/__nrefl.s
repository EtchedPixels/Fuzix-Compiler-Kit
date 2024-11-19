	.export __nref_4
	.export __nstore_4
	.export __nstore_4_0
	.export __nstore_4b


__nref_4:
	lda	*r13
	mov	a,r5
	decd	r13
	lda	*r13
	mov	a,r4
	decd	r13
	lda	*r13
	mov	a,r3
	decd	r13
	lda	*r13
	mov	a,r2
	rets

__nstore_4_0:
	clr	r5
__nstore_4b:
	clr	r4
	clr	r3
	clr	r2
__nstore_4:
	mov	r5,a
	sta	*r13
	decd	r13
	mov	r4,a
	sta	*r13
	decd	r13
	mov	r3,a
	sta	*r13
	decd	r13
	mov	r2,a
	sta	*r13
	rets