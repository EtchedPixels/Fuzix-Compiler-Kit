;
;	Shift ,X right by D
;
	.export __xshreqc
	.code

__xshreqc:
	clra
	andb	#7
	beq	nowork
	tfr	b,a
	ldb	,x
right8:
	asrb
	deca
	bne	right8
	stb	,x
	rts
nowork:
	ldb	,x
	rts
