;
;	Shift ,X left by D
;
	.export __xshleq
	.export __xshlequ
	.code

	.setcpu 6803

__xshleq:
__xshlequ:
	clra
	andb	#15
	beq	nowork
	cmpb	#8
	beq	bytemove
	bcc	fast
	stab	@tmp
	ldd	,x
left16:
	lslb
	rola
	dec	@tmp
	bne	left16
	std	,x
	rts
fast:
	andb	#7
	stab	@tmp
	ldaa	1,x
	clrb
left16f:
	lsla
	dec	@tmp
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
	
	