	.65c816
	.a16
	.i16

	.export __booll

__booll:
	stz @hireg
	ora @hireg
	beq done		; A already zero
	lda #1
done:	rts
