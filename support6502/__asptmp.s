;
;	Return XA = A + SP and write it to @tmp
;
;	Must preserve Y
;
	.export __asptmp
	.export __sptmp

	.code

__asptmp:
	ldx @sp+1
	clc
	adc @sp
	bcc l1	
	inx
l1:	stx @tmp+1
	sta @tmp
	rts

__sptmp:
	lda @sp
	ldx @sp+1
	sta @tmp
	stx @tmp+1
	rts

