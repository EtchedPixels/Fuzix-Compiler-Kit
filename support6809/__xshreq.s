;
;	Shift ,X left by D
;
	.export __xshreq
	.code

__xshreq:
	clra
	andb	#15
	beq	nowork
	tfr	d,y
	ldd	,x
right16:
	asra
	rorb
	ldy	-1,y
	bne	right16
	std	,x
	rts
nowork:
	ldd	,x
	rts
