;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccltl
	.export __ccltul

	.setcpu 6803

__ccltul:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	bhi	true
	beq	cclow
	bra	false
__ccltl:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	bgt	true
	bne	false
cclow:
	ldd	@tmp
	subd	4,x
	bhi	true
false:
	clra
	clrb
out:
	pulx
	ins
	ins
	ins
	ins
	tstb
	jmp	,x
true:
	ldd	@one
	bra	out
