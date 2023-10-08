	.65c816
	.a16
	.i16

	.export __gteqx

__gteqx:
	stx @tmp
	sec
	sbc @tmp
	bvc false
	lda #1
	rts
false:	lda #0
	rts

