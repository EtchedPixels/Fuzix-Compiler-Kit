	.65c816
	.a16
	.i16

	.export __gtequx

__gtequx:
	stx @tmp
	sec
	sbc @tmp
	bcs false
	lda #1
	rts
false:	lda #0
	rts

