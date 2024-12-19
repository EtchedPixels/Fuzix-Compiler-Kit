	.export __andeql

;	(TOS.L) & hireg:ea

__andeql:
	pop p3
	pop p2		; P2 points to our target
	push p3
	and a,0,p2
	xch a,e
	and a,1,p2
	xch a,e
	st ea,0,p2
	ld t,ea
	ld ea,:__hireg
	and a,2,p2
	xch a,e
	and a,3,p2
	xch a,e
	st ea,2,p2
	st ea,:__hireg
	ld ea,t
	ret
