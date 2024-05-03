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
	stab	@tmp
	beq	bytemove
	bcc	fast
	ldaa	,x
	ldab	1,x
right16:
	lsra
	rorb
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
	lsrb
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
	ldab	,x
	clra
	staa	,x
	stab	1,x
	rts

