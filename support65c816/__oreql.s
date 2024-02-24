	.export __oraeql

	.code

__oraeql:
	; X is the pointer
	; hireg:A is the value
	sta @tmp
	lda 0,x
	ora @tmp
	sta 0,x
	pha
	lda @hireg
	ora 2,x
	sta @hireg
	sta 2,x
	tax
	pla
	rts
