	.65c816
	.a16
	.i16

	.export __pluseqxc
	.export __pluseqxcu

__pluseqxc:
__pluseqxcu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	clc
	adc @tmp
	sta 0,x
	sep #0x20
	rts
