;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	TOS <= E
;
	.export __cclteqc

__cclteqc:
	st a,:__tmp
	pop p2
	pop ea
	sub a,:__tmp
	bz true
	bp positive
	ld a,s			;  S = 1
	and a,=0x40
	bz true			; S = 1 O = 0 true
false:	ld ea,=0		; S = 1 O = 1 false
	push p2
	ret
positive:
	ld a,s			; S = 0
	and a,=0x40
	bz false		; S = 0, O = 0 false
true:	ld ea,=1		; S = 0, O = 1 true
	push p2
	ret


