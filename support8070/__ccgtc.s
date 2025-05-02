;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	TOS > A
;
	.export __ccgtc

__ccgtc:
	st a,:__tmp
	pop p2
	pop ea
	sub a,:__tmp
	bz false
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


