;
;	XA is the pointer, add the amount given. These are used a lot
;

	.export	__plusplus2
	.code

__plusplus2:
	sta	@tmp
	stx	@tmp+1
	ldy	#1
	lda	(@tmp),y
	tax
	dey
	lda	(@tmp),y
	pha
	clc
	adc	#2
	sta	(@tmp),y
	txa
	adc	#0
	iny
	sta	(@tmp),y
	pla
	rts		; alwayus exits with Y = 1, XA old value

