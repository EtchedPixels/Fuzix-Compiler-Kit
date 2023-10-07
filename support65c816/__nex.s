	.65c816
	.a16
	.i16

	.export __nex
	.export __nexu

__nex:
__nexu:
	stx @tmp
	sec
	sbc @tmp
	beq false
	lda #1
false:	rts
