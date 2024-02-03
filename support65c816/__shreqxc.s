	.65c816
	.a16
	.i16

	.export __shreqxc

__shreqxc:
	; Shift [A] X times
	stx @tmp
	tax
	sep #0x20
	lda 0,x
	rep #0x20
	; Sign extend ?
	bpl sexp
	ora #0xFF00
doshifts:
	phx
	ldx @tmp
	jsr __rsx
	plx
	sep #0x20
	sta 0,x
	rep #0x20
	rts
sexp:
	and #0xFF
	bra doshifts
