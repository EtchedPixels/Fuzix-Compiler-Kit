	.65c816
	.a16
	.i16

	.export __remeqxc

	; A is ptr X is a value to divide by
__remeqxc:
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	sep #0x20
	phx
	ldx @tmp
	jsr __remx
	plx
	rep #0x20
	sta 0,x
	sep #0x20
	rts
