;
;	Unsigned 16x16 mul
;
	.export __mul
	.export __mulu
	.export __multmp

__mul:
__mulu:
	jsr	__poptmp
__multmp:
	; XA * tmp

	sta	@tmp1
	stx	@tmp1+1

	lda	#0
	sta	@tmp2+1
	ldy	#16

nextbit:
	asl	a
	rol	@tmp2+1

	rol	@tmp
	rol	@tmp+1

	bcc	noadd

	clc
	adc	@tmp1
	tax
	lda	@tmp1+1
	adc	@tmp2+1
	sta	@tmp2+1
	txa

noadd:	dey
	bne	nextbit
	ldx	@tmp2+1
	rts
