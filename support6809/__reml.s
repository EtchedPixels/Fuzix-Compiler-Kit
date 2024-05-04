	.export __reml
	.code

__reml:
	exg	d,y
	bita	#0x80
	beq	nosignfix
	exg	d,y
	lbsr	__negatel
	exg	d,y
nosignfix:
	sty	,--s
	std	,--s
	;
	;	Sign check
	;
	lda	6,s
	sta	@tmp4
	bpl	nosignfix2
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
	lbsr	div32x32
	;
	;	Get the result
	;
	ldy	@tmp2
	lda	@tmp4
	bpl	done
	ldd	@tmp3
	lbsr	__negatel
pop4:
	ldx	4,s
	leas	10,s
	jmp	,x
done:
	ldd	@tmp3
	bra	pop4
