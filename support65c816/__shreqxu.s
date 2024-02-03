	.65c816
	.a16
	.i16

	.export __shreqxu

__shreqxu:
	; Shift [A] X times
	stx @tmp
	tax
	lda 0,x
	phx
	ldx @tmp
	jsr __shrxu
	plx
	sta 0,x
	rts

