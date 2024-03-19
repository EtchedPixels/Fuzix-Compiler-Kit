;
;	,X over Y:D
;
	.export __xremequl
	.code
__xremequl:
	leas	-10,s		; Make space for the framr expected
	std	2,s
	sty	0,s
	ldd	,x
	std	6,s
	ldd	2,x
	std	8,s
	tfr	s,x
	stx	,--s
	jsr	div32x32
	ldy	@tmp2
	ldd	@tmp3
	ldx	,s++
	sty	,x
	std	2,x

	ldx	10,x
	leas	14,s
	jmp	,x
