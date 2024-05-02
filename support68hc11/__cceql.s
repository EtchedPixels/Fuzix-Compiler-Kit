;
;	Compare Y:D with TOS
;
	.export __cceql

__cceql:
	tsx
	cpy	2,x
	bne	false
	subd	4,x
	bne	false
	ldd	@one
out:
	puly
	pulx
	pulx
	tstb
	jmp	,y
false:
	clra
	clrb
	bra	out
