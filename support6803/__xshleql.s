;
;	TODO optimize
;
	.export __xshleql
	.export __xshlequl
	.code

	.setcpu 6803

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
	ldd 0,x
	std @hireg
	ldd 2,x
	rts
