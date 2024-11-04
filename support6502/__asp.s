;
;	Return XA = A + SP
;
	.export __asp

	.code

__asp:
	clc
	adc @sp
	pha
	bcc l1	
	inc @sp+1
l1:	tax
	pla
	rts
