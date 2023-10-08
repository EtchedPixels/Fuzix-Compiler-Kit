	.65c816
	.a16
	.i16

	.export __eqeqx
	.export __eqeqxu

__eqeqx:
__eqeqxu:
	stx @tmp
	sec
	sbc @tmp
	bne false
	inc a	; was 0 now 1 and ne
	rts
false:	lda #0
	rts
