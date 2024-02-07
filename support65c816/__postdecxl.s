	.65c816
	.a16
	.i16

	.export __postdecxl
	.export __postdecxul

;	(A) - hireg:x

__postdecxl:
__postdecxul:
	stx @tmp
	tax
	lda 0,x
	pha
	sec
	sbc @tmp
	sta 0,x
	lda 2,x
	pha
	sbc @hireg
	sta 2,x
	pla
	sta @hireg
	pla
	rts
