;
;	Compare hireg:EA with TOS
;
	.export __cceql

__cceql:
	sub ea,2,p1	; compare low words
	or a,e
	bnz false	; low match fail
	ld ea,:__hireg
	sub ea,4,p1	; compare high words
	or a,e
	bnz false
	ld ea,=1
out:
	pop p3
	pop p2
	pop p2
	push p3
	ret
false:
	ld ea,=0
	bra out
