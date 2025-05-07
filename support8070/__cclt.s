;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	@tmp < EA
;
;	N != V && !Z
;
	.export __cclttmp

__cclttmp:
	sub ea,:__tmp
	or a,e
	bz false
	xch a,e		;	get sign into A
	bp positive
	ld a,s		;	N = 1
	and a,=0x40
	bz true		;	N = 1 O = 0 true
false:	ld ea,=0	; 	N = 1 O = 1 false
	ret
positive:
	ld a,s			; N = 0
	and a,=0x40
	bz false		; N = 0, V = 0 false
true:	ld ea,=1		; N = 0, V = 1 true
	ret


