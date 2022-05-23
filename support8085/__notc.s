		.export	__notc

		.setcpu 8085
		.code

__notc:
		mov	a,l
		ora	a
		jz	__true
		jmp	__false
