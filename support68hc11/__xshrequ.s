;
;	Shift ,X right by D
;
	.export __xshrequ
	.code

__xshrequ:
	clra
	andb	#15
	beq	nowork
	cmpb	#8
	xgdy
	beq	bytemove
	bcc	fast
	ldd	,x
right16:
	lsra
	rorb
	dey
	bne	right16
	std	,x
	rts
fast:
	andb	#7
	xgdy
	ldab	,x
	clra
right16f:
	lsrb
	dey
	bne	right16f
	std	,x
	rts
nowork:
	ldd	,x
	rts
bytemove:
	ldab	,x
	clra
	std	,x
	rts

