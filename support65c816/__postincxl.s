; postincl/decl shouldn't be needed any more
	.65c816
	.a16
	.i16

	.export __postincxl
	.export __postincxul

;	(a) + hireg:x

__postincxl:
__postincxul:
	stx @tmp		; value to add
	tax
	lda 0,x			; variable low
	pha			; save as result
	clc
	adc @tmp		; new low
	sta 0,x
	lda 2,x			; get high
	pha			; save as result
	adc @hireg		; finish add
	sta 2,x			; save back
	pla			; get high word to hireg
	sta @hireg
	pla			; get low word to A
	rts
