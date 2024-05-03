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
	beq	bytemove
	bcc	fast
	stab	@tmp
	ldaa	,x
	ldab	1,x
right16:
	lslb
	rora
	dec	@tmp
	bne	right16
	staa	,x
	stab	1,x
	rts
fast:
	andb	#7
	stab	@tmp
	ldab	,x
	clra
right16f:
	lslb
	dec	@tmp
	bne	right16f
	staa	,x
	stab	1,x
	rts
nowork:
	ldaa	,x
	ldab	1,x
	rts
bytemove:
	ldab	1,x
	clra
	staa	,x
	stab	1,x
	rts
	
	