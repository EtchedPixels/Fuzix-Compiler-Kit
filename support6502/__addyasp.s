;
;	Add YA to SP (used for add and subtracts)
;
	.export __addyasp

	.code

__addyasp:
	clc
	adc	@sp
	sta	@sp
	tya
	adc	@sp+1
	sta	@sp+1
	rts

	