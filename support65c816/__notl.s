	.65c816
	.a16
	.i16

	.export __notl

__notl:
	ora @hireg
	beq true
	stz @hireg
	lda #0
	rts
true:
	stz @hireg
	inc a		; was 0 now 1
	rts
