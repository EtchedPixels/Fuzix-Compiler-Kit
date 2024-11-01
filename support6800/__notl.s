	.export __notl
	.code

__notl:
	orab	@hireg
	orab	@hireg+1
	bne	false
	tsta
	bne	false
	clra
	ldab	#1
	rts
false:
	clra
	clrb
	rts
