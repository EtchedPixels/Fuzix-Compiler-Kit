	.65c816
	.a16
	.i16

	.export __mulx
	.export __mulxu


__mulxu:
__mulx:	; calculate A * X
	stz @sum
	sta @tmp
loop:
	asl @sum
	lda @tmp
	bpl noadd
	clc
	txa
	adc @sum
	sta @sum
noadd:	asl @tmp
	bne loop
	rts
