;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgteql
	.export __ccgtequl

	.setcpu 6803

__ccgtequl:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	bhi	false
	blo	true
	bra	cclow
__ccgteql:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	bgt	false
cclow:
	ldd	@tmp
	subd	4,x
	bhi	false
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
