;
;	Shift ,X left by D
;
	.export __xshleqc
	.export __xshlequc
	.code

__xshleqc:
__xshlequc:
	clra
	andb	#7
	beq	nowork
	tba
	ldab	,x
left8:
	lslb
	deca
	bne	left8
	stab	,x
	rts
nowork:
	ldab	,x
	rts
