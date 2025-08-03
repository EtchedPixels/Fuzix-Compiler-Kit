;
;	(TOS) += hireg:XA
;	Result returned
;
	.export __pluseql

__pluseql:
	jsr	__poptmp
	; @tmp is now the pointer to the 32bit value
	ldy	#0
	clc
	adc	(@tmp),y
	sta	(@tmp),y
	pha
	iny
	txa
	adc	(@tmp),y
	sta	(@tmp),y
	tax
	iny
	lda	@hireg
	adc	(@tmp),y
	sta	(@tmp),y
	sta	@hireg
	iny
	lda	@hireg+1
	adc	(@tmp),y
	sta	(@tmp),y
	sta	@hireg+1
	pla
	rts
