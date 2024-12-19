	.export __oreql

;	(TOS.L) & hireg:ea

__oreql:
	pop p3
	pop p2		; P2 points to our target
	push p3
	or a,0,p2
	xch a,e
	or a,1,p2
	xch a,e
	st ea,0,p2
	ld t,ea
	ld ea,:__hireg
	or a,2,p2
	xch a,e
	or a,3,p2
	xch a,e
	st ea,2,p2
	st ea,:__hireg
	ld ea,t
	ret
