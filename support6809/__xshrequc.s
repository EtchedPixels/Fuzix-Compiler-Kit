;
;	Shift ,X right by D
;
	.export __xshrequc
	.code

__xshrequc:
	clra
	andb	#7
	beq	nowork
	tfr	b,a
	ldb	,x
right8:
	lsrb
	deca
	bne	right8
	stb	,x
	rts
nowork:
	ldb	,x
	rts
