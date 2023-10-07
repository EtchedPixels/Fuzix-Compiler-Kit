	.65c816
	.a16
	.i16

	.export __andeqx
	.export __andeqxu

__andeqx:
__andeqxu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	and @tmp
	sta 0,x
	rts
