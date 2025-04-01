;
;	If we better understood the actual limits on the 8070 MPY op we
;	could probably do much better.
;
	.export __mull
	.export __muleql

__mull:
	ld p3,=0
	push p3
	push p3		; make workspace

	xch ea,p2
	jsr slice

	ld ea,:__hireg
	xch ea,p2	; no ld p2,:__hireg
	jsr slice

	ld ea,2,p1
	st ea,:__hireg
	ld ea,0,p1

	pop p3		; remove workspace
	pop p3
	pop p2		; return addr
	pop p3
	pop p3		; argument
	push p2
	ret

;
;	Offsets at this point are
;
;	0,p1		return to mull
;	2,p1		low of working
;	4,p1		high of working
;	6,p1		return from mull to caller
;	8,p1		low of value
;	10,p1		high of value

;
slice:
	ld ea,=16
	st ea,:__tmp

	; Working value is in P2 at top of loop
next:
	ld ea,p2
	xch a,e		; test top bit of 16bits using bp
	bp noadd

	ld ea,2,p1	; do the addition for this cycle
	add ea,8,p1
	st ea,2,p1
	ld a,s
	bp skip		; no carry involved
	ld ea,4,p1
	add ea,=1	; carry into upper word
addh:
	add ea,10,p1
	st ea,4,p1	; save upper result
noadd:
	ld ea,8,p1	; start shifting
	add ea,8,p1	; use add so we get a carry indicator for shift
	st ea,8,p1	; save
	ld a,s
	bp nocarry	; did we carry ?
	ld ea,=1	; propogate carry into upper word
	add ea,10,p1
addh2:
	add ea,10,p1	; rotate left by including carry
	st ea,10,p1	; save upper word

	xch ea,p2	; get the working bits for the slice back
	sr ea		; rotate them
	xch ea,p2

	dld a,:__tmp
	bnz next
	ret
skip:
	add ea,4,p1
	bra addh
nocarry:
	ld ea,10,p1
	bra addh2

;
;	(*TOS) *= hireg:ea
;
__muleql:
	st ea,:__tmp
	ld ea,2,p1
	xch ea,p3
	; P3 now the pointer
	push p3		; save working ptr
	ld ea,2,p3
	push ea
	ld ea,0,p3
	push ea
	ld ea,:__tmp
	jsr __mull
	; This took the argument off the stack
	pop p3
	st ea,0,p3
	ld ea,:__hireg
	st ea,2,p3
	pop p2
	pop p3		; drop EA argument
	push p2
	ret
