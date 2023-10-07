	.65c816
	.a16
	.i16

	.export __minuseqx
	.export __minuseqxu

__minuseqx:
__minuseqxu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	sec
	sbc @tmp
	sta 0,x
	rts
