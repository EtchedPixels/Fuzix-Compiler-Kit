;
;	Shift ,X right by D
;
	.export __xshrequc
	.code

__xshrequc:
	clra
	andb	#7
	beq	nowork
	tba
	ldab	,x
right8:
	lsrb
	deca
	bne	right8
	stab	,x
	rts
nowork:
	ldab	,x
	rts
