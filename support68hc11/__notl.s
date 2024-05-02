	.export __notl
	.code
__notl:
	cpy	#0
	bne	true
	cmpa	#0
	bne	true
	cmpb	#0
	bne	true
	clra
	clrb
	rts
true:
	ldd	@one
	rts
