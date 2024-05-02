;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgteql
	.export __ccgtequl

__ccgtequl:
	tsx
	cpy	2,x
	bhi	false
	blo	true
	bra	cclow
__ccgteql:
	cpy	2,x
	bgt	false
cclow:
	subd	4,x
	bhi	false
true:
	ldd	@one
out:
	puly
	pulx
	pulx
	tstb
	jmp	,y
false:
	clra
	clrb
	bra	out
