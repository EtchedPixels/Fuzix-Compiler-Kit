;
;	Negate hireg:XA
;
	.export __negatel

__negatel:
	clc
	eor	#0xFF
	adc	#1
	pha
	txa
	eor	#0xFF
	adc	#0
	tax
	lda	@hireg
	eor	#0xFF
	adc	#0
	sta	@hireg
	lda	@hireg+1
	eor	#0xFF
	adc	#0
	sta	@hireg+1
	rts
