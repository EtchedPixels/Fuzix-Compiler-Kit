	.65c816
	.a16
	.i16

	.export __shreqxcu

__shreqxcu:
	; Shift [A] X times
	stx @tmp
	tax
	lda 0,x
	ldx @tmp
	phx
	jsr __shrxcu
	plx
	sta 0,x
	rts

