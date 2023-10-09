	.65c816
	.a16
	.i16

	.export __pluseqxl
	.export __pluseqxul

__pluseqxl:
__pluseqxul:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	clc
	adc @tmp
	sta 0,x
	pha
	lda 2,x
	adc @hireg
	sta 2,x
	pla
	rts
