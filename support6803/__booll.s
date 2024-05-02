	.export __booll
	.code

	.setcpu 6803

__booll:
	orab	@hireg
	orab	@hireg+1
	subd	#0
	beq	false	; If this passes then D is already zero
true:
	ldd	@one
false:
	rts
