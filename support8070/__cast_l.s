;
;	Integer to long cast
;
	.export __cast_l
	.export __cast_ul

__cast_l:
__cast_ul:
	xch a,e
	bp nowork
	ld t,ea
	ld ea,=0xFFFF
	st ea,:__hireg
	ld ea,t
nowork:
	xch a,e
	ret
