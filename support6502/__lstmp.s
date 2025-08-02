;
;	XA << @tmp
;
	.export __lstmp
	.export __lstmpu
	.export __l_ltlt

	.code

__l_ltlt:
	pha
	dey
	lda	(@sp),y
	sta	@tmp
	ldy	#0
	sty	@tmp+1
	pla
__lstmp:
__lstmpu:
	stx	@tmp+1
	ldx	@tmp
loop:	asl	a
	rol	@tmp+1
	dex
	bne	loop
	ldx	@tmp+1
	rts

	