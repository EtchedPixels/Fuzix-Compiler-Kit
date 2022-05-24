
			.export __castu_l
			.setcpu 8080
			.code

__castu_l:
	xchg
	lxi	h,0
	shld	__hireg
	xchg
	ret
