;
;	Shift ,X left by D
;
	.export __xshreq
	.code

	.setcpu 6803
__xshreq:
	clra
	andb	#15
	beq	nowork
	stab	@tmp
	ldd	,x
right16:
	asra
	rorb
	dec	@tmp
	bne	right16
	std	,x
	rts
nowork:
	ldd	,x
	rts
