;
;	If we better understood the actual limits on the 8070 MPY op we
;	could probably do much better.
;
	.export __mull

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

slice:
	ld ea,=16
	st ea,:__tmp

	xch ea,p2	; get working value into EA

next:
	ld p2,ea	; save working value
	xch a,e		; test top bit of 16bits using bp
	bp noadd

	ld ea,0,p1	; do the addition for this cycle
	add ea,4,p1
	st ea,0,p1
	ld a,s
	bp skip
	ld ea,2,p1
	add ea,=1
	add ea,2,p1
addh:
	add ea,6,p1
	st ea,2,p1
noadd:
	ld ea,4,p1
	sl ea
	st ea,4,p1
	ld a,s
	bp nocarry
	ld ea,=1
	add ea,6,p1
addh2:
	add ea,6,p1	; rotate left by including carry
	st ea,6,p1

	xch ea,p2	; get the working bits for the slice back
	sr ea		; rotate them

	dld a,:__tmp
	bnz next
	ret
skip:
	ld ea,2,p1
	bra addh
nocarry:
	ld ea,6,p1
	bra addh2