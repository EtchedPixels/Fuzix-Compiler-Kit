	.65c816
	.a16
	.i16

	.export __lteqxu

	; A v X
__lteqxu:
	stx @tmp
	cmp @tmp
	beq true
	bcc true
	lda #0
	rts
true:	lda #1
	rts
