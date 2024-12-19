;
;	Integer to long cast
;
	.export __cast_l
	.export __cast_ul

__cast_l:
__cast_ul:
	ld t,ea
	xch a,e
	bp clhigh
	ld ea,=0xFFFF
out:
	st ea,:__hireg
	ld ea,t
	ret
clhigh:
	ld ea,=0
	bra out
