;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __cclteql
	.export __ccltequl

	.setcpu 6803
__ccltequl:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	blo	false
	bhi	true
	bra	cclow
__cclteql:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	blt	false
	bgt	true
cclow:
	subd	4,x
	blo	false
true:
	ldd	@one
out:
	pulx
	ins
	ins
	ins
	ins
	tstb
	jmp	,x
false:
	clra
	clrb
	bra	out
