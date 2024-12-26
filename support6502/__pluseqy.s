;
;	(XA) += Y
;	returns XA = result
;

	.export __pluseq1
	.export __pluseq2
	.export __pluseq4
	.export __pluseqy

__pluseq4:
	ldy	#4
	jmp	__pluseqy
__pluseq2:
	ldy	#2
	.byte	0xCD	;	skip 2 bytes
__pluseq1:
	ldy	#1
__pluseqy:
	sty	@tmp1
	ldy	#0
	sty	@tmp1+1
	sta	@tmp
	stx	@tmp+1

	clc
	lda	(@tmp),y
	adc	@tmp1
	sta	(@tmp),y
	iny
	pha
	lda	(@tmp),y
	adc	@tmp1+1
	sta	(@tmp),y
	tax
	pla
	rts
