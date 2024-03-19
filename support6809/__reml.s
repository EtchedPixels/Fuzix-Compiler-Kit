	.export __reml
	.code

__reml:
	exg	d,y
	sta	,-s		; Save as we need to sign check at end
	bita	#0x80
	bpl	nosignfix
	exg	d,y
	jsr	__negatel
	exg	d,y
nosignfix:
	std	,--s
	sty	,--s
	;
	;	Sign check
	;
	lda	6,x
	bpl	nosignfix2
	ldd	8,x
	subd	#1
	std	8,x
	ldd	6,x
	sbcb	#0
	sbca	#0
	coma
	comb
	std	6,x
	com	8,x
	com	9,x
nosignfix2:
	tfr	s,d
	jsr	div3x32
	;
	;	Get the result
	;
	ldy	@tmp2
	lda	,-s
	bpl	done
	ldd	@tmp3
	jsr	__negatel
pop4:
	ldx	4,s
	leas	10,s
	jmp	,x
done:
	ldd	@tmp3
	bra	pop4
