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
	tfr	d,y
	beq	bytemove
	bcc	fast
	ldd	,x
right16:
	lsra
	rorb
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
	lsrb
	ldy	-1,y
	bne	right16f
	std	,x
	rts
nowork:
	ldd	,x
	rts
bytemove:
	ldb	,x
	clra
	std	,x
	rts

