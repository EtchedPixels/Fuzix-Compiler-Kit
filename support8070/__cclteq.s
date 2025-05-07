;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	TOS <= EA
;
;	N != V | Z
;
	.export __cclteqtmp

__cclteqtmp:
	sub ea,:__tmp
	or a,e
	bz true
	xch a,e			; get sign into A
	bp positive
	ld a,s			; N = 1, so S needs to be 0
	and a,=0x40
	bz true			;
false:	ld ea,=0		;
	ret
positive:
	ld a,s			; N = 0 so S needs to be 1
	and a,=0x40
	bz false		; S = 0, O = 0 false
true:	ld ea,=1		; S = 0, O = 1 true
	ret


