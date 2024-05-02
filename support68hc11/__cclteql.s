;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __cclteql
	.export __ccltequl

__ccltequl:
	tsx
	cpy	2,x
	blo	false
	bhi	true
	bra	cclow
__cclteql:
	tsx
	cpy	2,x
	blt	false
	bgt	true
cclow:
	subd	4,x
	blo	false
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
