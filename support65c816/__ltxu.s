	.65c816
	.a16
	.i16
	.export __ltxu

__ltxu:
	stx @tmp
	sec
	sbc @tmp
	bcc true
	lda #0
	rts
true:	lda #1
	rts
