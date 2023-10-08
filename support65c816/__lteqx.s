	.65c816
	.a16
	.i16

	.export __lteqx

	; A v X
__lteqx:
	stx @tmp
	cmp @tmp
	beq true
	bvc true
	lda #0
	rts
true:	lda #1
	rts
