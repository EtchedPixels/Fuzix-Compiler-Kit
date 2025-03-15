	.export __div
	.export __divtmp
	.export __rem
	.export __remtmp

__div:
	jsr	__poptmp
__divtmp:
	ldy	#0
	jsr	neg
	sta	@tmp1
	stx	@tmp1+1
	lda	@tmp
	ldx	@tmp+1
	jsr	neg
	sta	@tmp
	stx	@tmp+1
	sty	@tmp3
	jsr	__dodivu
	;	Result is now in XA
	ror	@tmp3
	bcc	ispve
	jsr	doneg
ispve:	rts

__rem:
	jsr	__poptmp
__remtmp:
	jsr	neg
	sta	@tmp1
	stx	@tmp1+1
	lda	@tmp
	ldx	@tmp+1
	ldy	#0
	jsr	neg
	sta	@tmp
	stx	@tmp+1
	sty	@tmp3
	jsr	__dodivu
	;	Result is now in @tmp2
	ldx	@tmp2+1
	lda	@tmp2
	ror	@tmp3
	bcc	ispve
	jsr	doneg
	rts

;
;	Turn a value positive if needed. Count sign changes in Y
;
neg:
	cpx	#0x00
	bpl	nowork
	iny
doneg:
	clc
	eor	#0xFF
	adc	#1
	pha
	txa
	eor	#0xFF
	adc	#0
	tax
	pla
nowork:
	rts
	