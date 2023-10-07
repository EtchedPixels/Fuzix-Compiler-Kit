	.65c816
	.a16
	.i16

	.export __pluseqx
	.export __pluseqxu

__pluseqx:
__pluseqxu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	clc
	adc @tmp
	sta 0,x
	rts
