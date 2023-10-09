; postincl/decl shouldn't be needed any more
	.65c816
	.a16
	.i16

	.export __postincl

;	(top of stack) - hireg:a

__postincl:
	sta @tmp
	pla		; return
	plx		; pointer
	pha
	clc
	lda 0,x
	pha
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
