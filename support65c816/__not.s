	.65c816
	.a16
	.i16

	.export __not

__notc:
	and #0xFF
__not:
	tax
	lda #1
	cpx #0
	beq done
	txa
done:	rts
