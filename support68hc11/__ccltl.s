;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccltl
	.export __ccltul

__ccltul:
	tsx
	cpy	2,x
	bhi	true
	beq	cclow
	bra	false
__ccltl:
	cpy	2,x
	bgt	true
	bne	false
cclow:
	subd	4,x
	bhi	true
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
