; postincl/decl shouldn't be needed any more
	.65c816
	.a16
	.i16

	.export __postincxl
	.export __postincxul

;	(x) - hireg:a

__postincxl:
__postincxul:
	sta @tmp
	lda 0,x
	pha
	clc
	adc @tmp
	sta 0,x
	lda 2,x
	pha
	adc @hireg
	sta 2,x
	pla
	sta @hireg
	pla
	rts
