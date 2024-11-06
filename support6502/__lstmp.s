;
;	XA << @tmp
;
	.export __lstmp
	.export __lstmpu

	.code

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

	