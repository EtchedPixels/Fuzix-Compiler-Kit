	.export __booll
	.code
__booll:
	cmpy	#0
	bne	false
	cmpd	#0
	bne	false
	ldd	@one
	rts
false:
	clra
	clrb
	rts
