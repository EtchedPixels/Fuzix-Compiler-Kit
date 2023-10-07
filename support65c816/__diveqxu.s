	.65c816
	.a16
	.i16

	.export __diveqxu

	; A is ptr X is a value to divide by
__diveqxu:
	stx @tmp
	tax
	lda 0,x
	phx
	ldx @tmp
	jsr __divxu
	plx
	sta 0,x
	rts
