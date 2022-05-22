		.export __cmpltu
		.export __cmpltub
		.setcpu 8080
		.code

		; true if HL < DE
__cmpltub:
		mvi	h,0
__cmpltu:
		mov	a,h
		cmp	d
		jc	__true
		jnz	__false
		mov	a,l
		cmp	e
		jc	__true
		jmp	__false
