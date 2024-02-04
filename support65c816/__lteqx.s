	.65c816
	.a16
	.i16

	.export __lteqx

	; A v X
__lteqx:
	stx @tmp
	clc
	sbc @tmp
	bvc t1
	eor #0x8000
t1:	bpl false
	lda #1
	rts
false:	lda #0
	rts
