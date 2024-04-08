	.export __reml
	.code

;
;	FIXME: relies on accidentally useful value of X!
;
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
	lda	6,s
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
	jsr	div32x32
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
