;
;	Shift ,X right by D
;
	.export __xshleq
	.export __xshlequ
	.code

__xshleq:
__xshlequ:
	clra
	andb	#15
	beq	nowork
	cmpb	#8
	xgdy
	beq	bytemove
	bcc	fast
	ldd	,x
right16:
	lslb
	rora
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
	lslb
	dey
	bne	right16f
	std	,x
	rts
nowork:
	ldd	,x
	rts
bytemove:
	ldab	1,x
	clra
	std	,x
	rts
	
	