	.export __xoreql

;	(TOS.L) & hireg:ea

__xoreql:
	pop p3
	pop p2		; P2 points to our target
	push p3
	xor a,0,p2
	xch a,e
	xor a,1,p2
	xch a,e
	st ea,0,p2
	ld t,ea
	ld ea,:__hireg
	xor a,2,p2
	xch a,e
	xor a,3,p2
	xch a,e
	st ea,2,p2
	st ea,:__hireg
	ld ea,t
	ret
