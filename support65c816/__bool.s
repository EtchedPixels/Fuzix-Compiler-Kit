	.65c816
	.a16
	.i16

	.export __bool
	.export __boolc

__boolc:
	and #0xFF
__bool:
	tax
	lda #1
	cpx #0
	bne done
	txa		;  X is 0
done:	rts
