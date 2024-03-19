	.export __divul
	.code
__divul:
	std	,--s
	sty	,--s
	tfr	s,x
	jsr	div32x32
	ldy	6,x
	ldd	8,x
	ldx	4,x
	leas	10,s
	jmp	,x
