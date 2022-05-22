		.export	__notl

		.setcpu 8085
		.code

__notl:
		mov	a,h
		ora	l
		lhld	__hireg
		ora	h
		ora	l
		jz	__true
		jmp	__false
