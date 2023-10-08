	.65c816
	.a16
	.i16

	.export __plusl
	.export __popret

__plusl:
	plx
	sta @tmp
	clc
	pla
	adc @tmp
	sta @tmp
	pla
	adc @hireg
	sta @hireg
	lda @tmp
	phx
	rts
