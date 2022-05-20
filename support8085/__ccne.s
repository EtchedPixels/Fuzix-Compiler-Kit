		.export __ccne

		.setcpu 8085
		.code

__ccne:		xchg
		pop	h
		shld	__retaddr
		mov	a,l
		cmp	e
		jnz	__rtrue
		mov	a,h
		cmp	d
		jnz	__rfalse
		jmp	__rtrue
