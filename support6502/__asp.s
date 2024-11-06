;
;	Return XA = A + SP
;
;	Must preserve Y and @tmp
;
	.export __asp

	.code

__asp:
	ldx @sp+1
	clc
	adc @sp
	bcc l1	
	inx
l1:	rts
