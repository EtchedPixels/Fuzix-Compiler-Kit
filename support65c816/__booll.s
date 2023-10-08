	.65c816
	.a16
	.i16

	.export __booll

__booll:
	ldx #0
	stx @hireg
	ora @hireg
	beq done		; A already zero
	lda #1
done:	rts
