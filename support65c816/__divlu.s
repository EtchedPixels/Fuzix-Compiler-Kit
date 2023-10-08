	.65c816
	.a16
	.i16

	.export __divlu
	.export __modlu

__divlu:
	dey
	dey
	dey
	dey
	sta 0,y
	lda @hireg
	sta 2,y
	jsr div32x32
	lda 6,y
	sta @hireg
	lda 4,y
popit:
	iny
	iny
	iny
	iny
	iny
	iny
	iny
	iny
	rts

__modlu:
	dey
	dey
	dey
	dey
	sta 0,y
	lda @hireg
	sta 2,y
	jsr div32x32
	lda @tmp3
	sta @hireg
	lda @tmp2
	bra popit
