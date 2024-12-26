
	.export __pluseq

	; (TOS) - EA
__pluseq:
	jsr	__poptmp
	; poptmp set Y to 0
	clc
	adc	(@tmp),y
	sta	(@tmp),y
	pha
	iny
	txa
	adc	(@tmp),y
	sta	(@tmp),y
	tax
	pla
	rts
