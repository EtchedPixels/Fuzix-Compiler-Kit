;
;	,X - Y:D, return new ,X
;
	.export __xminuseql
	.export __xminusequl

	.code

__xminuseql:
__xminusequl:
	staa	@tmp
	stab	@tmp+1
	ldaa	2,x
	ldab	3,x
	subb	@tmp+1
	sbca	@tmp
	staa	2,x
	stab	3,x
	ldaa	@hireg
	ldab	@hireg+1
	staa	@tmp
	stab	@tmp+1
	ldaa	,x
	ldab	1,x
	sbcb	@tmp+1
	sbca	@tmp
	staa	,x
	stab	1,x
	staa	@hireg
	stab	@hireg+1
	ldaa	2,x
	ldab	3,x
	rts
