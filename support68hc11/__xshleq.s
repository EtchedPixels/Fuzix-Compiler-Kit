;
;	Shift ,X left by D
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
left16:
	lslb
	rola
	dey
	bne	left16
	std	,x
	rts
fast:
	andb	#7
	xgdy
	ldaa	1,x
	clrb
left16f:
	lsla
	dey
	bne	left16f
	std	,x
	rts
nowork:
	ldd	,x
	rts
bytemove:
	ldaa	1,x
	clrb
	std	,x
	rts
	
	