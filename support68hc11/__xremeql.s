;
;	,X over Y:D
;
	.export __xremeql
	.code
__xremeql:
;
;	No nice way to do this
;
	pshs	x
	leas	-4,s		; make a frame
	pshs	d
	ldd	,x		; copy argument into frame
	std	4,s
	ldd	2,x
	std	2,s
	puls	d
	;	Y:D		is the divisor
	jsr	__reml
	leas	4,s		; throw frame
	puls	x
	sty	0,x
	std	2,x
	rts
