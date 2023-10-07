	.65c816
	.a16
	.i16

	.export __oreqxc
	.export __oreqxcu

__oreqxc:
__oreqxcu:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	ora @tmp
	sta 0,x
	sep #0x20
	rts
