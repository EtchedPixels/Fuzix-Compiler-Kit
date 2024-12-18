;
;	Compare hireg:EA with TOS
;
	.export __ccnel

__ccnel:
	sub ea,2,p1	; compare low words
	or a,e
	bnz true	; low match fail
	ld ea,:__hireg
	sub ea,4,p1	; compare high words
	or a,e
	bnz true
	; EA is already 0
out:
	pop p3
	pop p2
	pop p2
	push p3
	ret
true:
	ld ea,=1
	bra out
