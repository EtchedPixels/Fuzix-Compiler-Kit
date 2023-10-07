	.65c816
	.a16
	.i16

	.export __andeqxc
	.export __andeqxcu

__andeqxc:
__andeqxcu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	and @tmp
	sta 0,x
	sep #0x20
	rts
