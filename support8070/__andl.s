	.export __bandl

__bandl:
	and a,2,p1
	xch a,e
	and a,3,p1
	xch a,e
	ld t,ea
	ld ea,:__hireg
	and a,4,p1
	xch a,e
	and a,5,p1
	xch a,e
	st ea,:__hireg
	ld ea,t
	pop p3
	pop p2
	pop p2
	push p3
	ret
