;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccltl
	.export __ccltul

__ccltul:
	cmpy	2,s
	blo	true
	bra	cclow
__ccltl:
	cmpy	2,s
	blt	true
cclow:
	cmpd	4,s
	blo	true
	clra
	clrb
out:
	ldx	,x
	leas	4,s
	jmp	,x
true:
	ldd	@one
	bra	out
