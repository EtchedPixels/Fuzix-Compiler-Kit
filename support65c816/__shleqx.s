	.65c816
	.a16
	.i16

	.export __shleqx
	.export __shleqxu

__shleqx:
__shleqxu:
	; Shift [A] X times
	stx @tmp
	tax
	lda 0,x
	ldx @tmp
	phx
	jsr __shlx
	plx
	sta 0,x
	rts

