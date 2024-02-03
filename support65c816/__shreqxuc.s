	.65c816
	.a16
	.i16

	.export __shreqxuc

__shreqxuc:
	; Shift [A] X times
	stx @tmp
	tax
	sep #0x20
	lda 0,x
	rep #0x20
	ldx @tmp
	and #0xFF
	phx
	jsr __rsxu
	plx
	sep #0x20
	sta 0,x
	rep #0x20
	rts

