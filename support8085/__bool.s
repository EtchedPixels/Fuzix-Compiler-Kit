		.export	__bool
		.export __cmpne0

		.setcpu 8085
		.code

__cmpne0:
__bool:
		mov	a,h
		ora	l
		lxi	h,0
		rz
		inx	h
		ret
