;
;	,X over Y:D
;
	.export __xdiveql
	.code
__xdiveql:
;
;	No nice way to do this
;
	pshs	x
	leas	-4,s		; make a frame
	pshs	d
	ldd	,x		; copy argument into frame
	std	2,s
	ldd	2,x
	std	4,s
	puls	d
	;	Y:D		is the divisor
	lbsr	__divl
;	leas	4,s		; throw frame (called code did this)
	puls	x
	sty	0,x
	std	2,x
	rts
