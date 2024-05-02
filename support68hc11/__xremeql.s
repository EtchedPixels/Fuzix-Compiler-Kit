;
;	,X over Y:D
;
	.export __xremeql
	.code
__xremeql:
;
;	No nice way to do this
;
	pshx			; save pointer
	pshx			; make frame
	pshx
	pshy			; save working high
	pshb			; save working low
	psha
	tsy
	ldd	,x		; copy argument into frame
	std	4,y
	ldd	2,x
	std	6,y
	pula
	pulb
	puly			; get Y : D back
	tsx			; frame pointer
	;	Y:D		is the divisor
	jsr	__reml
	pulx
	pulx			; throw frame
	pulx			; destination
	sty	0,x
	std	2,x
	rts
