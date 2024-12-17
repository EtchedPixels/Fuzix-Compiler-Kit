;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	TOS > EA
;
	.export __ccgt

__ccgt:
	st ea,:__tmp
	pop p2
	pop ea
	sub ea,:__tmp
	or a,e
	bz false
	xch a,e			; get sign into A
	bp positive
	ld a,s
	and a,=0x40
	bz false
true:	ld ea,=1
	push p2
	ret
positive:
	ld a,s
	and a,=0x40
	bz true
false:	ld ea,=0
	push p2
	ret


