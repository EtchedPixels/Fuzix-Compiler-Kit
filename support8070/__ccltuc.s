
;	TOS < A

	.export __ccltuc

__ccltuc:
	sub a,2,p1		; calc EA - TOS
	; if A <= TOS
	bz false		; TOS = EA
	rrl a			; carry to top of A
	bp false		; carry set if TOS > EA
true:
	ld ea,=1
	pop p2
	pop p3
	push p2
	ret
false:
	ld ea,=0
	pop p2
	pop p3
	push p2
	ret
