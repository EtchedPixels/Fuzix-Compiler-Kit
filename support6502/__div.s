	.export __div
	.export __divtmp
	.export __rem
	.export __remtmp

;
;	TOS / XA
;
__div:
	jsr	__poptmp
	; Now @tmp/XA
__divtmp:
	; Y is used to count sign swaps
	ldy	#0
	jsr	neg
	; Ensure XA is positive
	sta	@tmp1
	stx	@tmp1+1
	; Save XA into @tmp1
	lda	@tmp
	ldx	@tmp+1
	; Get the dividend
	jsr	neg
	sta	@tmp
	stx	@tmp+1
	; Again fix the sign up and track in Y
	sty	@tmp3
	; Do the unsigned divide
	jsr	__dodivu
	;	Result is now in XA
	ror	@tmp3
	bcc	ispve
	;	Fix the resulting sign up
	jsr	doneg
ispve:	rts

__rem:
	jsr	__poptmp
	; Now @tmp % XA
__remtmp:
	; Fix up the sign of XA
	jsr	neg
	; Save XA
	sta	@tmp1
	stx	@tmp1+1
	; Get the divisor and sort out the sign with tracking
	lda	@tmp
	ldx	@tmp+1
	ldy	#0
	jsr	neg
	sta	@tmp
	stx	@tmp+1
	sty	@tmp3
	; Do the unsigned maths
	jsr	__dodivu
	;	Result is now in @tmp2
	ldx	@tmp2+1
	lda	@tmp2
	ror	@tmp3
	bcc	ispve
	jmp	doneg

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
	