	.65c816
	.a16
	.i16

	.export __gteqxu

__gteqxu:
	stx @tmp
	sec
	sbc @tmp
	lda #0		; move carry into A
	rol a
	rts

