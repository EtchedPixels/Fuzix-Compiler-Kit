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
	beq	bytemove
	bcc	fast
	stab	@tmp
	ldaa	,x
	ldab	1,x
left16:
	lslb
	rola
	dec	@tmp
	bne	left16
	staa	,x
	stab	1,x
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
	staa	,x
	stab	1,x
	rts
nowork:
	ldaa	,x
	ldab	1,x
	rts
bytemove:
	ldaa	1,x
	clrb
	staa	,x
	stab	1,x
	rts
	
	