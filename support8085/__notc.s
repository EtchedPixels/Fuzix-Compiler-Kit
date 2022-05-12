		.export	__notc

		.setcpu 8085
		.code

__notc:
		mov	a,l
		ora	a
		lxi	h,0
		rnz
		inx	h
		ret
