;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgteql
	.export __ccgtequl

__ccgtequl:
	cmpy	2,s
	bhi	false
	blo	true
	bra	cclow
__ccgteql:
	cmpy	2,s
	bgt	false
cclow:
	cmpd	4,s
	bhi	false
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
