;
;	,X over Y:D
;
	.export __xdiveql
	.code

	.setcpu 6803

__xdiveql:
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
	; Stack the base value
	; Recover hireg:D
	ldd	@tmp
	;	hireg:D		is the divisor
	jsr	__divl
	;	This pulled 4 bytes
	pulx	; destination back
	std	2,x
	ldd	@hireg
	std	0,x
	ldd	2,x
	rts
