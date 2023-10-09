	.65c816
	.a16
	.i16

	.export __minuseqxl
	.export __minuseqxul

__minuseqxl:
__minuseqxul:
	; A is the pointer X is the value - backwards to how we want!
	stx @tmp
	tax
	lda 0,x
	sec
	sbc @tmp
	sta 0,x
	pha
	lda 2,x
	sbc @hireg
	sta 2,x
	sta @hireg
	pla
	rts
