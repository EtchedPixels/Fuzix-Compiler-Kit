;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgtl
	.export __ccgtul

__ccgtul:
	cmpy	2,s
	blo	true
	beq	cclow
	bra	false
__ccgtl:
	cmpy	2,s
	blt	true
	bne	false
cclow:
	cmpd	4,s
	blo	true
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
