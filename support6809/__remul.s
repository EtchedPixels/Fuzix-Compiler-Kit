	.export __remul
	.code
__remul:
	std	,--s
	sty	,--s
	tfr	s,x
	lbsr	div32x32
	; Result is in tmp2/tmp3
	ldy	@tmp2
	ldd	@tmp3
	ldx	4,x
	leas	10,s
	jmp	,x
