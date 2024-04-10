;
;	,X over Y:D
;
	.export __xdivequl
	.code
__xdivequl:
	leas	-10,s		; Make space for the frame expected
	std	2,s
	sty	0,s
	ldd	,x
	std	6,s
	ldd	2,x
	std	8,s
	stx	,--s
	leax	2,s
	jsr	div32x32
	ldy	6,x
	ldd	8,x
	ldx	,s++
	sty	,x
	std	2,x

	leas	10,s
	rts
