		.export __cmpltequ
		.export __cmpltequb
		.setcpu 8080
		.code

		; true if HL <= DE

__cmpltequb:
		mvi	h,0
__cmpltequ:
		mov	a,h
		cmp	d
		jc	__true
		jnz	__false
		mov	a,l
		cmp	e
		jz	__true
		jnc	__false
		jmp	__true
