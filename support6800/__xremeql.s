;
;	,X over Y:D
;
	.export __xremeql
	.code

__xremeql:
;
;	No nice way to do this
;
	stx	@tmp1		; save pointer
	staa	@tmp
	stab	@tmp+1
	ldaa	2,x
	ldab	3,x
	pshb
	psha
	ldaa	,x
	ldab	1,x
	pshb
	psha
	ldaa	@tmp
	ldab	@tmp+1
	;	hireg:D		is the divisor
	jsr	__reml
	ldx	@tmp1		; destination
	staa	2,x
	stab	3,x
	ldaa	@hireg
	staa	0,x
	ldaa	@hireg+1
	staa	1,x
	ldaa	2,x
	rts
