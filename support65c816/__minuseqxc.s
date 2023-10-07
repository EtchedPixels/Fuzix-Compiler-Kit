	.65c816
	.a16
	.i16

	.export __minuseqxc
	.export __minuseqxcu

__minuseqxc:
__minuseqxcu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	rep #$20
	lda 0,x
	sec
	sbc @tmp	; little endian so this is fine
	sta 0,x
	sep #$20
	rts
