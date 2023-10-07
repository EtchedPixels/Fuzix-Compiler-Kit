	.65c816
	.a16
	.i16

	.export __remeqxu

	; A is ptr X is a value to divide by
__remeqxu:
	stx @tmp
	tax
	lda 0,x
	phx
	ldx @tmp
	jsr __remxu
	plx
	sta 0,x
	rts
