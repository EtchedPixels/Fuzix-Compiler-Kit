
	.export __nref_1
	.export __nref_2
	.export __nstore_1
	.export __nstore_2
	.export __nstore_1_0
	.export __nstore_2_0
	.export __nstore_2b

	;	r12 is the pointer to the last byte

__nref_1:
	lda	*r13
	mov	a,r5
	clr	r4
	rets

__nref_2:
	lda	*r13
	mov 	a,r5
	decd	r13
	lda	*r13
	mov	a,r4
	rets

__nstore_1_0:
	clr	r5
__nstore_1:
	mov	r5,a
	sta	*r13
	rets

__nstore_2_0:
	clr	r5
__nstore_2b:
	clr	r4
__nstore_2:
	mov	r5,a
	sta	*r13
	decd	r13
	mov	r4,a
	sta	*r13
	rets
