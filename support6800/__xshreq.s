;
;	Shift ,X left by D
;
	.export __xshreq
	.code

__xshreq:
	clra
	andb	#15
	beq	nowork
	stab	@tmp
	ldaa	,x
	ldab	1,x
right16:
	asra
	rorb
	dec	@tmp
	bne	right16
	staa	,x
	stab	1,x
	rts
nowork:
	ldaa	,x
	ldab	1,x
	rts
