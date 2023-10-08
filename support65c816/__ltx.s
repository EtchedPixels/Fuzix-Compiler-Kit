	.65c816
	.a16
	.i16

	.export __ltx

__ltx:
	stx @tmp
	sec
	sbc @tmp
	bvc true
	lda #0
	rts
true:	lda #1
	rts

