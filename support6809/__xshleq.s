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
	tfr	d,y
	beq	bytemove
	bcc	fast
	ldd	,x
right16:
	lslb
	rora
	ldy	-1,y
	bne	right16
	std	,x
	rts
fast:
	andb	#7
	tfr	d,y
	ldb	,x
	clra
right16f:
	lslb
	ldy	-1,y
	bne	right16f
	std	,x
	rts
nowork:
	ldd	,x
	rts
bytemove:
	ldb	1,x
	clra
	std	,x
	rts
	
	