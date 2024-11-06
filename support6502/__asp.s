;
;	Return XA = A + SP
;
;	Must preserve Y and @tmp
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
