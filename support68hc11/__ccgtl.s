;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgtl
	.export __ccgtul

__ccgtul:
	tsx
	cpy	2,x
	blo	true
	beq	cclow
	bra	false
__ccgtl:
	cpy	2,x
	blt	true
	bne	false
cclow:
	subd	4,x
	blo	true
false:
	clra
	clrb
out:
	puly
	pulx
	pulx
	tstb
	jmp	,y
true:
	ldd	@one
	bra	out
