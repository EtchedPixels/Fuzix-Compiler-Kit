	.65c816
	.a16
	.i16

	.export __mulxeq
	.export __mulxequ

	; A is ptr X is a value
__mulxequ:
__mulxeq:
	stx @tmp
	tax
	lda 0,x
	phx
	ldx @tmp
	jsr __mulx
	plx
	sta 0,x
	rts
