		.export __cmpne
		.export __cmpneb

		.setcpu 8080
		.code

;
;	Tighter version with the other value in DE
;
__cmpneb:
		mvi	h,0
__cmpne:
		mov	a,l
		cmp	e
		jnz	__true
		mov	a,h
		cmp	d
		jnz	__true
		jmp	__false
