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
	tfr	d,y
	beq	bytemove
	bcc	fast
	ldd	,x
left16:
	lslb
	rola
	leay	-1,y
	bne	left16
	std	,x
	rts
fast:
	andb	#7
	tfr	d,y
	lda	1,x
	clrb
left16f:
	lsla
	leay	-1,y
	bne	left16f
	std	,x
	rts
nowork:
	ldd	,x
	rts
bytemove:
	lda	,x
	clrb
	std	,x
	rts
	
	