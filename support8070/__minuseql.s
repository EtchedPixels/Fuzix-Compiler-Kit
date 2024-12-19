;
;	(TOS) - hireg:ea
;
	.export __minuseql

__minuseql:
	pop p3
	pop p2
	push p3
	st ea,:__tmp
	ld ea,0,p2
	sub ea,:__tmp
	st ea,0,p2
	ld t,ea
	; Low half done
	rrl a
	bp carry		; need to carry
	ld ea,2,p2
hi:
	sub ea,:__hireg
	st ea,2,p2
	st ea,:__hireg
	ld ea,t
	ret
carry:
	ld ea,2,p2
	sub ea,=1
	bra hi
