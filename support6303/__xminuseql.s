;
;	,X - Y:D, return new ,X
;
	.export __xminuseql
	.export __xminusequl

	.code
	.setcpu 6803

__xminuseql:
__xminusequl:
	std	@tmp
	ldd	2,x
	subd	@tmp
	std	2,x
	ldd	@hireg
	std	@tmp
	ldd	,x
	sbcb	@tmp+1
	sbca	@tmp
	std	,x
	std	@hireg
	ldd	2,x
	rts
