;
;	(TOS) -= hireg:XA
;	Result returned
;
	.export __minuseql

__minuseql:
	jsr	__poptmp
	; @tmp is now the pointer to the 32bit value
	ldy	#0
	sec
	sta	@tmp2
	lda	(@tmp),y
	sbc	@tmp2
	sta	(@tmp),y
	pha
	iny
	stx	@tmp2
	lda	(@tmp),y
	sbc	@tmp2
	sta	(@tmp),y
	tax
	iny
	lda	(@tmp),y
	sbc	@hireg
	sta	(@tmp),y
	sta	@hireg
	iny
	lda	(@tmp),y
	adc	@hireg+1
	sta	(@tmp),y
	sta	@hireg+1
	pla
	rts
