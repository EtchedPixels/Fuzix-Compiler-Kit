;
;		H holds the pointer
;
		.export __derefl
		.setcpu	8080
		.code

__derefl:
		push	b
		mov	c,m
		inx	h
		mov	b,m
		inx	h
		mov	e,m
		inx	h
		mov	d,m
		xchg
		shld	__hireg
		mov	l,c
		mov	h,b
		pop	b
		ret
