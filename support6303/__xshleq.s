;
;	Shift ,X right by D
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
right16:
	lslb
	rora
	dec	@tmp
	bne	right16
	std	,x
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
	
	