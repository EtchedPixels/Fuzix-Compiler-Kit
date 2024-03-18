	.export __notl
	.code
__notl:
	cmpy	#0
	bne	true
	cmpd	#0
	bne	true
	clra
	clrb
	rts
true:
	ldd	@one
	rts
