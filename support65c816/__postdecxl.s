	.65c816
	.a16
	.i16

	.export __postdecxl
	.export __postdecxul

;	(X) - hireg:a

__postdecxl:
__postdecxul:
	sta @tmp
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
