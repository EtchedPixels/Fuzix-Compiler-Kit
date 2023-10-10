	.65c816
	.a16
	.i16

	.export __shreqxc

__shreqxc:
	; Shift [A] X times
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	sep #0x20
	ldx @tmp
	phx
	jsr __rsx
	plx
	rep #0x20
	sta 0,x
	sep #0x20
	rts

