	.65c816
	.a16
	.i16

	.export __cpll

__cpll:
	eor #0xFFFF
	pha
	lda #0xFFFF
	eor @hireg
	sta @hireg
	pla
	rts
