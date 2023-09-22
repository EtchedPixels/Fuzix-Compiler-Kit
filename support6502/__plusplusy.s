;
;	XA is the pointer, add the amount given. The other 8bit cases are
;	not such a hot path
;
	.globl	__plusplus4
	.globl	__plusplusy
	.text

__plusplus4:
	ldy	#4
__plusplusy:
	sty	@tmp1
	sta	@tmp
	stx	@tmp+1
	ldy	#1
	ldx	(@tmp),y
	dey
	lda	(@tmp),y
	pha
	clc
	adc	(@tmp1),y
	sta	(@tmp),y
	bcc	l1
	inx
	iny
	inc	(@tmp1),y
	dey
l1:	pla
	rts
