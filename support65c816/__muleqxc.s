	.65c816
	.a16
	.i16

	.export __muleqxc
	.export __mulxeqcu

	; A is ptr X is a value
__muleqxcu:
__muleqxc:
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	sep #0x20
	phx
	ldx @tmp
	jsr __mulx
	plx
	rep #0x20
	sta 0,x
	sep #0x20
	rts
