		.export __cmpgtequ
		.export __cmpgtequb
		.setcpu 8080
		.code

		; true if HL >= DE

__cmpgtequb:
		mvi	h,0
__cmpgtequ:
		mov	a,h
		cmp	d
		jc	__false
		jnz	__true
		mov	a,l
		cmp	e
		jc	__false
		jmp	__true
