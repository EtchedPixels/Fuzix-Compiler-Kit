;
;	Compare Y:D with TOS
;
	.export __ccnel

	.setcpu 6803

__ccnel:	
	tsx
	subd	4,x
	bne	true
	ldd	@hireg
	subd	2,x
	bne	true
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
