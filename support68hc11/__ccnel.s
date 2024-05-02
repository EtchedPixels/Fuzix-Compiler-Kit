;
;	Compare Y:D with TOS
;
	.export __ccnel

__ccnel:	
	tsy
	cpy	2,y
	bne	true
	subd	4,y
	bne	true
	clra
	clrb
out:
	puly
	pulx
	pulx
	tstb
	jmp	,y
true:
	ldd	@one
	bra	out
