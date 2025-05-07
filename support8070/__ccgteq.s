;
;	The 8070 does have both OV and CY flags but they are clunky to
;	access
;
;	TOS >= EA
;
;	N != V
;
	.export __ccgteqtmp

__ccgteqtmp:
	sub ea,:__tmp
	or a,e
	bz true
	xch a,e			; get sign into A
	bp positive
	ld a,s			; N = 1
	and a,=0x40
	bz false		; N != V false
true:	ld ea,=1
	ret
positive:
	ld a,s
	and a,=0x40		; N = 0
	bz true			; N == V true
false:	ld ea,=0
	ret


