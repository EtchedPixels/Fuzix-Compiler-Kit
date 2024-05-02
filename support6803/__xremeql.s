;
;	,X over Y:D
;
	.export __xremeql
	.code

	.setcpu 6803
__xremeql:
;
;	No nice way to do this
;
	pshx			; save pointer
	std	@tmp
	ldd	2,x
	pshb
	psha
	ldd	,x
	pshb
	psha
	ldd	@tmp
	;	hireg:D		is the divisor
	jsr	__reml
	pulx			; destination
	std	2,x
	ldd	@hireg
	std	0,x
	ldd	2,x
	rts
