	.export __adceql

	.code

	; += helper for 32bit
__adceql:
	; X is the pointer
	; hireg:A is the value
	sta @tmp
	lda 0,x
	clc
	adc @tmp
	sta 0,x
	pha
	lda @hireg
	adc 2,x
	sta @hireg
	sta 2,x
	tax
	pla
	rts
