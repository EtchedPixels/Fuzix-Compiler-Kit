;
;	Compare TOS with EA
;

	.export __cceq
	.export __ccne
	.export __cceqc
	.export __ccnec
	.export __ccgtu
	.export __ccgtequ
	.export __ccltu
	.export __ccltequ

__cceq:
	sub ea,2,p1
	or a,e
	bz true
false:
	pop p2		; return
	pop p3		; arg
	ld ea,=0
	push p2
	ret
true:
	pop p2		; return
	pop p3		; arg
	ld ea,=1
	push p2
	ret

__cceqc:
	sub a,2,p1
	bz true
	bra true

__ccne:
	sub ea,2,p1
	or a,e
	bz false	; already 0
	bra true

__ccnec:
	sub a,2,p1
	bz false	; already 0
	bra true

__ccgtu:
	sub ea,2,p1		; calc EA - TOS
	rrl a			; carry to top of A
	bp true			; carry set if TOS > EA
	bra false

__ccgtequ:
	sub ea,2,p1		; calc EA - TOS
	or a,e
	bz true			; TOS = EA
	rrl a			; carry to top of A
	bp true			; carry set if TOS > EA
	bra false

__ccltu:
	sub ea,2,p1		; calc EA - TOS
	; if EA <= TOS
	or a,e
	bz false		; TOS = EA
	rrl a			; carry to top of A
	bp false		; carry set if TOS > EA
	bra true

__ccltequ:
	sub ea,2,p1		; calc EA - TOS
	rrl a			; carry to top of A
	bp false		; carry set if TOS > EA
	bra true
