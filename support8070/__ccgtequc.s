
;	TOS >= EA

	.export __ccgtequc

__ccgtequc:
	sub a,2,p1		; calc EA - TOS
	or a,e
	bz true			; TOS = EA
	rrl a			; carry to top of A
	bp true			; carry set if TOS > EA
false:
	ld ea,=0
	pop p2
	pop p3
	push p2
	ret
true:
	ld ea,=1
	pop p2
	pop p3
	push p2
	ret
