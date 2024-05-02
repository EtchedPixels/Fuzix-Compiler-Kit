;
;	Shift ,X left by D
;
	.export __xshreq
	.code

__xshreq:
	clra
	andb	#15
	beq	nowork
	xgdy
	ldd	,x
right16:
	asra
	rorb
	dey
	bne	right16
	std	,x
	rts
nowork:
	ldd	,x
	rts
