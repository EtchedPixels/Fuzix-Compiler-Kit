;
;	Shift ,X right by D
;
	.export __xshreqc
	.code

__xshreqc:
	clra
	andb	#7
	beq	nowork
	tba
	ldab	,x
right8:
	asrb
	deca
	bne	right8
	stab	,x
	rts
nowork:
	ldab	,x
	rts
