;
;	,X over Y:D
;
	.export __xdiveql
	.code
__xdiveql:
;
;	No nice way to do this
;
	pshx			; make frame
	pshx
	pshx
	pshy
	pshb
	psha
	tsy
	ldd	,x		; copy argument into frame
	std	4,y
	ldd	2,x
	std	6,y
	pula
	pulb
	puly
	;	Y:D		is the divisor
	jsr	__divl
	;	Called function threw frame
	pulx			; destination back
	sty	0,x
	std	2,x
	rts
