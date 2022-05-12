		.export	__negate
		.export __cpl

		.setcpu 8085
		.code

__negate:
		dcx	h
__cpl:
		mov	a,h
		cma
		mov	h,a
		mov	a,l
		cma
		mov	l,a
		ret
