;
;	Compare Y:D with TOS
;
	.export __cceql

	.setcpu 6803
__cceql:
	tsx
	subd	4,x
	bne	false
	ldd	@hireg
	subd	2,x
	bne	false
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
