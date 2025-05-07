;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	TOS > EA
;
;	N == V & !Z
;
	.export __ccgttmp

__ccgttmp:
	sub ea,:__tmp
	or a,e
	bz false		; !Z
	xch a,e
	bp positive		; check OV matches sign (0 0)
	ld a,s
	and a,=0x40		; V = 1 S = 1 is true
	bz false
true:	ld ea,=1
	ret
positive:
	ld a,s
	and a,=0x40		; Check V and sign both 0
	bz true
false:	ld ea,=0
	ret


