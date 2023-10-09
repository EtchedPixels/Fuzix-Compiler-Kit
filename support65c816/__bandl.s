	.65c816
	.a16
	.i16

	.export __bandl

__bandl:
	plx
	sta @tmp
	pla
	and @tmp
	sta @tmp
	pla
	and @hireg
	sta @hireg
	lda @tmp
	phx
	rts
