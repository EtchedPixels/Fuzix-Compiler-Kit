		.export __cmpeq
		.export __cmpeqb

		.setcpu 8085
		.code

;
;	Tighter version with the other value in DE
;
__cmpeqb:
		mvi	h,0
__cmpeq:
		mov	a,l
		cmp	e
		jnz	__false
		mov	a,h
		cmp	d
		jnz	__false
		jmp	__true
