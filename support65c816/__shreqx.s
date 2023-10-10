	.65c816
	.a16
	.i16

	.export __shreqx

__shreqx:
	; Shift [A] X times
	stx @tmp
	tax
	lda 0,x
	ldx @tmp
	phx
	jsr __rsx
	plx
	sta 0,x
	rts

