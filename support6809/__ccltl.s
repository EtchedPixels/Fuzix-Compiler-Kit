;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccltl
	.export __ccltul

__ccltul:
	cmpy	2,s
	bhi	true
	beq	cclow
	bra	false
__ccltl:
	cmpy	2,s
	bgt	true
	bne	false
cclow:
	cmpd	4,s
	bhi	true
false:
	clra
	clrb
out:
	ldx	,s
	leas	6,s
	tstb
	jmp	,x
true:
	ldd	@one
	bra	out
