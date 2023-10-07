	.65c816
	.a16
	.i16

	.export __oreqx
	.export __oreqxu

__oreqx:
__oreqxu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	ora @tmp
	sta 0,x
	rts
