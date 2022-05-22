		.export __cmpeq0
		.export __cmpeq0b
		.setcpu	8080
		.code

__cmpeq0b:
		mvi	h,0
__cmpeq0:
		mov	a,h
		ora 	l
		lxi 	h,0
		jnz	ret0
		inr	l
		ret	; nz
ret0:		xra 	a
		ret
