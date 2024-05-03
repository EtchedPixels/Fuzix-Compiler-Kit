	.export __booll
	.code

__booll:
	orab	@hireg
	orab	@hireg+1
	cmpa	#0
	beq	false	; If this passes then D is already zero
	cmpb	#0
	beq	false
true:
	clra
	ldab	@one
	rts
false:
	clra
	clrb
	rts
