	.export __booll
	.code

__booll:
	orab	@hireg
	orab	@hireg+1
	bne		true
	tsta
	beq	false	; If this passes then D is already zero
true:
	clra
	ldab	#1
	rts
false:
	clra
	clrb
	rts
