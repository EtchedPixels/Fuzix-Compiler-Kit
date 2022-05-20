		.export __cceq

		.setcpu 8085
		.code

__cceq:		xchg
		pop	h
		shld	__retaddr
		mov	a,l
		cmp	e
		jnz	__rfalse
		mov	a,h
		cmp	d
		jnz	__rfalse
		jmp	__rtrue
