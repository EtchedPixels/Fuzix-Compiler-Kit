;
;	If we better understood the actual limits on the 8070 MPY op we
;	could probably do much better.
;
	.export __mull
	.export __muleql

__mull:
	push	ea	;	save low bits
	ld	ea,=0
	push	ea
	push	ea	;	create workspace for working value

	ld	a,=32
	st	a,:__tmp

;	0,p1		low of working
;	2,p1		high of working
;	4,p1		low of other argument
;	6,p1		return
;	8,p1		low of argument
;	10,p1		high of argument
;
;	@__hireg:	32bit other argument
;
domul:
	; 32bit shift left of working
	ld	ea,0,p1
	add	ea,0,p1
	st	ea,0,p1
	ld	a,s
	bp	nocarry1
	ld	ea,2,p1
	add	ea,=1
cdone1:
	add	ea,2,p1
	st	ea,2,p1
	; 32bit shift left of argument
	ld	ea,8,p1
	add	ea,8,p1
	st	ea,8,p1
	ld	a,s
	bp	nocarry2
	ld	ea,10,p1
	add	ea,=1
cdone2:
	add	ea,10,p1
	st	ea,10,p1
	;	Was the top bit set before the shift
	ld	a,s
	bp	no_add
	;	Add to the working value
	ld	ea,0,p1
	add	ea,4,p1
	st	ea,0,p1
	ld	a,s
	bp	nocarry3
	ld	ea,2,p1
	add	ea,=1
cdone3:
	add	ea,:__hireg
	st	ea,2,p1
no_add:
	dld	a,:__tmp
	bnz	domul
	; Result is now on stack
	pop	p2		; result low into p2
	pop	ea		; result high
	st	ea,:__hireg
	xch	ea,p2		; Low word into EA
	pop	p2		; drop saved low word
	pop	p3		; Return
	pop	p2		; Drop argument
	pop	p2
	push	p3		; And done
	ret

nocarry1:
	ld	ea,2,p1
	bra	cdone1
nocarry2:
	ld	ea,10,p1
	bra	cdone2
nocarry3:
	ld	ea,2,p1
	bra	cdone3
;
;	(*TOS) *= hireg:ea
;
__muleql:
	st	ea,:__tmp
	ld	ea,2,p1
	xch	ea,p3
	; P3 now the pointer
	push	p3		; save working ptr
	ld	ea,2,p3
	push	ea
	ld	ea,0,p3
	push	ea
	ld	ea,:__tmp
	jsr	__mull
	; This took the argument off the stack
	pop	p3
	st	ea,0,p3
	ld	t,ea
	ld	ea,:__hireg
	st	ea,2,p3
	ld	ea,t
	pop	p2
	pop	p3		; drop address argument
	push	p2
	ret
