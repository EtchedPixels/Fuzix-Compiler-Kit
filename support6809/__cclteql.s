;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __cclteql
	.export __ccltequl

__ccltequl:
	cmpy	2,s
	blo	false
	bhi	true
	bra	cclow
__cclteql:
	cmpy	2,s
	blt	false
	bgt	true
cclow:
	cmpd	4,s
	blo	false
true:
	ldd	@one
out:
	ldx	,s
	leas	6,s
	tstb
	jmp	,x
false:
	clra
	clrb
	bra	out
