;
;		H holds the pointer
;
		.export __dereffsp
		.export __dereff
		.export __dereflsp
		.export __derefl
		.setcpu	8080
		.code

__dereffsp:
__dereflsp:
		dad	sp
__dereff:
__derefl:
		mov	e,m
		inx	h
		mov	d,m
		inx	h
		push	d
		mov	e,m
		inx	h
		mov	d,m
		xchg
		shld	__hireg
		pop	h
		ret
