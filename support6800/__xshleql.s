;
;	TODO optimize
;
	.export __xshleql
	.export __xshlequl
	.code

__xshleql:
__xshlequl:
	clra
	andb #31
	beq load_only
shlp:
	lsl 3,x
	rol 2,x
	rol 1,x
	rol 0,x
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
