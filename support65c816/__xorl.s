	.65c816
	.a16
	.i16

	.export __xorl

__xorl:
	plx
	sta @tmp
	pla
	eor @tmp
	sta @tmp
	pla
	eor @hireg
	sta @hireg
	lda @tmp
	phx
	rts
