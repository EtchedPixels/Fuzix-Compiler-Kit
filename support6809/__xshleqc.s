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
	tfr	b,a
	ldb	,x
left8:
	lslb
	deca
	bne	left8
	stb	,x
	rts
nowork:
	ldb	,x
	rts
