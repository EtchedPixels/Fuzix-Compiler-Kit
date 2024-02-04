	.65c816
	.a16
	.i16

	.export __gteqx

__gteqx:
	stx @tmp
	sec
	sbc @tmp
	bvc t1
	eor #0x8000
t1:	bpl true
	lda #0
	rts
true:	lda #1
	rts

