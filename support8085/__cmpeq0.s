		.export __cmpeq0
		.code

__cmpeq0:
		mov	a,h
		ora 	l
		lxi 	h,0
		jz	ret0
		inr	l
		ret	; nz
ret0:		xra 	a
		ret
