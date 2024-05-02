;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgtl
	.export __ccgtul

	.setcpu 6803
__ccgtul:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	blo	true
	beq	cclow
	bra	false
__ccgtl:
	tsx
	std	@tmp
	ldd	@hireg
	subd	2,x
	blt	true
	bne	false
cclow:
	ldd	@tmp
	subd	4,x
	blo	true
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
