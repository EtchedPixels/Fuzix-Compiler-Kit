		.export	__not

		.setcpu 8085
		.code

__not:
		mov	a,h
		ora	l
		lxi	h,0
		rnz
		inx	h
		ret
