	.export __eoreql

	.code

__eoreql:
	; X is the pointer
	; hireg:A is the value
	sta @tmp
	lda 0,x
	eor @tmp
	sta 0,x
	pha
	lda @hireg
	eor 2,x
	sta @hireg
	sta 2,x
	tax
	pla
	rts
