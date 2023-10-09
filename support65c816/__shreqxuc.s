	.65c816
	.a16
	.i16

	.export __shreqxuc

__shreqxuc:
	; Shift [A] X times
	stx @tmp
	tax
	lda 0,x
	ldx @tmp
	phx
	jsr __shrxuc
	plx
	sta 0,x
	rts

