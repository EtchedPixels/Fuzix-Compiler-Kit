	.export __booll
	.code
__booll:
	cpy	#0
	bne	true
	subd	#0
	beq	false	; If this passes then D is already zero
true:
	ldd	@one
false:
	rts
