	.65c816
	.a16
	.i16

	.export __gtx

	; A v X
__gtx:
	stx @tmp
	cmp @tmp
	beq false
	bvc false
	lda #1
	rts
false:	lda #0
	rts
