		.export	__bool
		.export __cmpne0

		.setcpu 8085
		.code

__cmpne0:	; a compare to non zero is a bool op
__bool:
		mov	a,h
		ora	l
		lxi	h,0
		rz
		inr	l		; NZ
		ret
