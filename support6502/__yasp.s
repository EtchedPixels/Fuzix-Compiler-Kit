;
;	Return XA = YA + SP
;
	.export __yasp

	.code

__yasp:
	clc
	adc @sp
	pha
	tya
	adc @sp+1
	tax
	pla
	rts