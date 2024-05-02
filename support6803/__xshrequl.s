;
;	TODO optimize
;
	.export __xshrequl
	.code

	.setcpu 6803
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
	ldd 0,x
	std @hireg
	ldd 2,x
	rts
