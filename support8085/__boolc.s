		.export	__boolc

		.setcpu 8085
		.code

__boolc:
		mov	a,l
		ora	a
		lxi	h,0
		rz
		inx	h
		ret
