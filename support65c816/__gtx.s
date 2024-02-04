	.65c816
	.a16
	.i16

	.export __gtx

	; A v X
__gtx:
	stx @tmp
	clc
	sbc  @tmp
	bvc t1
	eor #0x8000
t1:	bpl true
	lda #0
	rts
true:	lda #1
	rts
