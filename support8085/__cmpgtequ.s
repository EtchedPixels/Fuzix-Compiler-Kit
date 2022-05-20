		.export __cmpgtequ
		.setcpu 8080
		.code

		; true if HL >= DE

__cmpgtequ:
		mov	a,h
		cmp	d
		jc	__false
		jnz	__true
		mov	a,l
		cmp	e
		jc	__false
		jmp	__true
