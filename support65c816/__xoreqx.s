	.65c816
	.a16
	.i16

	.export __xoreqx
	.export __xoreqxu

__xoreqx:
__xoreqxu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	eor @tmp
	sta 0,x
	rts
