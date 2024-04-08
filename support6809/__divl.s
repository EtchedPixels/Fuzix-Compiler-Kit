	.export __divl
	.code

__divl:
	clr	@tmp4		; Sign tracking
	exg	d,y
	bita	#0x80
	bpl	nosignfix
	exg	d,y
	jsr	__negatel
	inc	@tmp4
	exg	d,y
nosignfix:
	std	,--s
	sty	,--s
	;
	;	Now do other argument
	;
	lda	6,s
	bpl	nosignfix2
	inc	@tmp4
	ldd	8,s
	subd	#1
	std	8,s
	ldd	6,s
	sbcb	#0
	sbca	#0
	coma
	comb
	std	6,s
	com	8,s
	com	9,s
nosignfix2:
	tfr	s,x
	jsr	div32x32
	ldb	@tmp4
	rorb
	bcc	nosignfix3
	ldy	6,x
	ldd	8,x
	jsr	__negatel
out:
	ldx	4,s
	leas	10,s
	jmp	,x
nosignfix3:
	ldy	6,x
	ldd	8,x
	bra	out
