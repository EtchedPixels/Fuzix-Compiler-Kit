	.65c816
	.a16
	.i16

	.export __postdecl

;	(top of stack) - hireg:a

__postdecl:
	sta @tmp
	pla		; return
	plx		; pointer
	pha
	sec
	lda 0,x
	pha
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
