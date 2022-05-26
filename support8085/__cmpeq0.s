		.export __cmpeq0
		.export __cmpeq0b
		.setcpu	8080
		.code

__cmpeq0:
		mov	a,h
		ora 	l
		jnz	__false
		jmp	__true
__cmpeq0b:
		mov	a,l
		ora	a
		jnz	__false
		jmp	__true
