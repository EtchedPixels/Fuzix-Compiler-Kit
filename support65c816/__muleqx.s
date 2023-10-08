	.65c816
	.a16
	.i16

	.export __muleqx
	.export __muleqxu

	; A is ptr X is a value
__muleqxu:
__muleqx:
	stx @tmp
	tax
	lda 0,x
	phx
	ldx @tmp
	jsr __mulx
	plx
	sta 0,x
	rts
