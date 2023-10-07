	.65c816
	.a16
	.i16

	.export __diveqx

	; A is ptr X is a value to divide by
__diveqx:
	stx @tmp
	tax
	lda 0,x
	phx
	ldx @tmp
	jsr __divx
	plx
	sta 0,x
	rts
