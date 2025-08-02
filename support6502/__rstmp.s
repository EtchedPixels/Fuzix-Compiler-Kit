;
;	XA >> @tmp
;
;	TODO; consider checking 8+ shift and byteswap but not clear
;	it's worth it
;
	.export __rstmpu
	.export __rstmp
	.export __l_gtgt
	.export __l_gtgtu

	.code

__l_gtgt:
	pha
	dey
	lda	(@sp),y
	sta	@tmp
	ldy	#0
	sty	@tmp
	pla
__rstmp:
	stx	@tmp+1
	cpx	#$80
	bcc	__rsneg
	; Positive and unsigned
__rstmpu:
	ldx	@tmp
loop:	lsr	@tmp+1
	ror	a
	dex
	bne	loop
	ldx	@tmp+1
	rts
	; Negative
__rsneg:
	ldx	@tmp
loopn:	sec
	ror	@tmp+1
	ror	a
	dex
	bne	loopn
	ldx	@tmp+1
	rts

__l_gtgtu:
	pha
	dey
	lda	(@sp),y
	sta	@tmp
	ldy	#0
	sty	@tmp
	pla
	jmp	__rstmpu
