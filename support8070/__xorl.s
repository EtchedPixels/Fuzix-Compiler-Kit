	.export __xorl

__xorl:
	xor a,2,p1
	xch a,e
	xor a,3,p1
	xch a,e
	ld t,ea
	ld ea,:__hireg
	xor a,4,p1
	xch a,e
	xor a,5,p1
	xch a,e
	st ea,:__hireg
	ld ea,t
	pop p3
	pop p2
	pop p2
	push p3
	ret
