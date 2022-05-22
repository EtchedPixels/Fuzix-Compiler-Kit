		.export	__bool
		.export __cmpne0
		.export __cmpne0b

		.setcpu 8080
		.code

__cmpne0b:
		mvi	h,0
__cmpne0:	; a compare to non zero is a bool op
__bool:
		mov	a,h
		ora	l
		lxi	h,0
		rz
		inr	l		; NZ
		ret
