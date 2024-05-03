;
;	,X over Y:D
;
	.export __xdiveql
	.code

__xdiveql:
;
;	No nice way to do this
;
	stx	@tmp1		; save pointer
	staa	@tmp
	stab	@tmp+1
	ldaa	2,x
	ldab	4,x
	pshb
	psha
	ldaa	,x	
	ldab	1,x
	pshb
	psha
	; Stack the base value
	; Recover hireg:D
	ldaa	@tmp
	ldab	@tmp+1
	;	hireg:D		is the divisor
	jsr	__divl
	;	This pulled 4 bytes
	ldx	@tmp1	; destination back
	staa	2,x
	stab	3,x
	ldaa	@hireg
	staa	0,x
	ldaa	@hireg+1
	staa	1,x
	ldaa	2,x
	rts
