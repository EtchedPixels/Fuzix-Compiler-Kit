;
;	Compare @TMP with EA
;

	.export __cceqtmp
	.export __cceqtmpu
	.export __ccnetmp
	.export __ccnetmpu
	.export __cceqtmpc
	.export __cceqtmpuc
	.export __ccnetmpc
	.export __ccnetmpuc
	.export __ccgttmpu
	.export __ccgteqtmpu
	.export __cclttmpu
	.export __cclteqtmpu

__cceqtmp:
__cceqtmpu:
	sub ea,:__tmp
	or a,e
	bz true
false:
	ld ea,=0
	ret
true:
	ld ea,=1
	ret

__cceqtmpc:
__cceqtmpuc:
	sub a,:__tmp
	bz true
	bra false

__ccnetmp:
__ccnetmpu:
	sub ea,:__tmp
	or a,e
	bz false	; already 0
	bra true

__ccnetmpc:
__ccnetmpuc:
	sub a,:__tmp
	bz false	; already 0
	bra true

__cclttmpu:
	sub ea,:__tmp		; calc EA - TOS
	ld a,s
	bp true			; carry set if TOS > EA
	bra false

__cclteqtmpu:
	sub ea,:__tmp		; calc EA - TOS
	or a,e
	bz true			; TOS = EA
	ld a,s
	bp true			; carry set if TOS > EA
	bra false

__ccgttmpu:
	sub ea,:__tmp		; calc EA - TOS
	; if EA <= TOS
	or a,e
	bz false		; TOS = EA
	ld a,s
	bp false		; carry set if TOS > EA
	bra true

__ccgteqtmpu:
	sub ea,:__tmp		; calc EA - TOS
	ld a,s
	bp false		; carry set if TOS > EA
	bra true
