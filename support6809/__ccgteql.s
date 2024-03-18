;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgteql
	.export __ccgtequl

__ccgtequl:
	cmpy	2,s
	blo	false
	bra	cclow
__ccgteql:
	cmpy	2,s
	blt	false
cclow:
	cmpd	4,s
	blo	false
	ldd	@one
out:
	ldx	,x
	leas	4,s
	jmp	,x
false:
	clra
	clrb
	bra	out
