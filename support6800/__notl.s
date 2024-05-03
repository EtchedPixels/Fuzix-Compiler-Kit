	.export __notl
	.code

__notl:
	orab	@hireg
	orab	@hireg+1
	bne	false
	cmpa	#0
	bne	false
	clra
	ldab	#1
	rts
false:
	clra
	clrb
	rts
