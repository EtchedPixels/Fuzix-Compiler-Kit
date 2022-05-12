		.export	__bool

		.setcpu 8085
		.code

__bool:
		mov	a,h
		ora	l
		lxi	h,0
		rz
		inx	h
		ret
