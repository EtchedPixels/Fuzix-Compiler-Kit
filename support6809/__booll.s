	.export __booll
	.code
__booll:
	cmpy	#0
	bne	true
	cmpd	#0
	beq	false	; If this passes then D is already zero
true:
	ldd	@one
false:
	rts
