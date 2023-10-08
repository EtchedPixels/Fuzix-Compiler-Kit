	.65c816
	.a16
	.i16

	.export __gtxu

	; A v X
__gtxu:
	stx @tmp
	cmp @tmp
	beq false
	bcc false
	lda #1
	rts
false:	lda #0
	rts
