;
;	,X - Y:D, return old ,X
;
	.export __xminuseql
	.export __xminusequl

	.code
__xminuseql:
__xminusequl:
	; TODO: re-entrant version ?
	std	@tmp
	ldd	2,x
	subd	@tmp
	std	2,x
	xgdy
	std	@tmp
	ldd	,x
	sbcb	@tmp+1
	sbca	@tmp
	std	,x
	xgdy
	rts
