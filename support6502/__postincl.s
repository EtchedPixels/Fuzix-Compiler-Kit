;
;	(TOS) += hireg:XA
;	Old contents of (TOS) returned
;
;	Can probably be optimized somewhat
;
	.export __postincl

__postincl:
	jsr	__poptmp
	; @tmp is now the pointer to the 32bit value
	ldy	#0
	clc
	sta	@tmp2
	lda	(@tmp),y
	pha
	adc	@tmp2
	sta	(@tmp),y
	pha
	iny
	stx	@tmp2
	lda	(@tmp),y
	tax
	adc	@tmp2
	sta	(@tmp),y
	iny
	lda	(@tmp),y
	sta	@tmp2
	adc	@hireg
	sta	(@tmp),y
	iny
	lda	(@tmp),y
	sta	@tmp2+1
	adc	@hireg+1
	sta	(@tmp),y
	lda	@tmp2
	sta	@hireg
	lda	@tmp2+1
	sta	@hireg+1
	pla
	rts
