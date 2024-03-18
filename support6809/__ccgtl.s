;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgtl
	.export __ccgtul

__ccgtul:
	cmpy	2,s
	bhi	true
	bra	cclow
__ccgtl:
	cmpy	2,s
	bgt	true
cclow:
	cmpd	4,s
	bhi	true
	clra
	clrb
out:
	ldx	,s
	leas	6,s
	jmp	,x
true:
	ldd	@one
	bra	out
