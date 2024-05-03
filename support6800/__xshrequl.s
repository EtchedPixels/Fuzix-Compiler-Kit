;
;	TODO optimize
;
	.export __xshrequl
	.code

__xshrequl:
	clra
	andb #31
	beq load_only
shlp:
	lsr 0,x
	ror 1,x
	ror 2,x
	ror 3,x
	decb
	bne shlp
load_only:
	ldaa 0,x
	ldab 1,x
	staa @hireg
	stab @hireg+1
	ldaa 2,x
	ldab 3,x
	rts
