	.65c816
	.a16
	.i16

	.export __orl

__orl:
	plx
	sta @tmp
	pla
	ora @tmp
	sta @tmp
	pla
	ora @hireg
	sta @hireg
	lda @tmp
	phx
	rts
