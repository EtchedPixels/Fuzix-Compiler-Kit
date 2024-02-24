	.export __andeql

	.code

__andeql:
	; X is the pointer
	; hireg:A is the value
	sta @tmp
	lda 0,x
	and @tmp
	sta 0,x
	pha
	lda @hireg
	and 2,x
	sta @hireg
	sta 2,x
	tax
	pla
	rts
