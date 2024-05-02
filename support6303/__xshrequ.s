;
;	Shift ,X right by D
;
	.export __xshrequ
	.code

	.setcpu 6803

__xshrequ:
	clra
	andb	#15
	beq	nowork
	cmpb	#8
	stab	@tmp
	beq	bytemove
	bcc	fast
	ldd	,x
right16:
	lsra
	rorb
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
	lsrb
	dec	@tmp
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

