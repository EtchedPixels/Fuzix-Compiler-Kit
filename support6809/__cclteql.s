;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __cclteql
	.export __ccltequl

__ccltequl:
	cmpy	2,s
	bhi	false
	bra	cclow
__cclteql:
	cmpy	2,s
	bgt	false
cclow:
	cmpd	4,s
	bhi	false
	ldd	@one
out:
	ldx	,x
	leas	4,s
	jmp	,x
false:
	clra
	clrb
	bra	out
