;
;		True if TOS < HL
;
		.export __ccltu

		.setcpu 8085

		.code
;
__ccltu:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		lxi	h,0
		rnc
		inr	l
		ret
