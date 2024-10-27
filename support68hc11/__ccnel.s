;
;	Compare Y:D with TOS
;
	.export __ccnel

__ccnel:	
	tsx
	cpy	2,x
	bne	true
	subd	4,x
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
