		.export	__not

		.setcpu 8085
		.code

__not:
		mov	a,h
		ora	l
		jz	__true
		jmp	__false
